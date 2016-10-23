#include "sandbox.h"

#include <iostream>
#include <string>
#include <array>
#include <system_error>
#include <vector>
#include <cstring>

#include <boost/format.hpp>
#include <boost/filesystem.hpp>
#include <yaml-cpp/yaml.h>

#include <wait.h>
#include <signal.h>
#include <seccomp.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ptrace.h>
#include <sys/reg.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <fcntl.h>
#include <pwd.h>

#include "utils.h"

using std::system_error;
using std::runtime_error;
using std::system_category;
using std::string;
using boost::filesystem::path;
using boost::format;

enum LogLevel {
    Debug, Info, Warning, Error
};
struct RunResult {
    RunStatus Status;
    // Return code if exited, Signal if signaled, Syscall number if killed by bad syscall.
    int Code;
    // Memory usage in KBytes
    int Memory;
    // Time in MS
    int Time;
};

void Log(format message, LogLevel level) {
    if (level >= Debug)
        std::cerr << message << std::endl;
}

const int Seccomp_DisabledSyscall = 0x1926;
const int Seccomp_ExecvpSyscall = 0x0817;
const int DefaultErrorCode = 89;
const int Seccomp_brk = 64;

// Seccomp: restrict syscalls
void InitalizeSeccomp() {
    scmp_filter_ctx ctx;

    try {
        Log(format("Child: Setting Seccomp options"), Debug);
        ctx = CheckNull(seccomp_init(SCMP_ACT_TRACE(Seccomp_DisabledSyscall)));

        Ensure_Seccomp(seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(rt_sigreturn), 0));
        Ensure_Seccomp(seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(exit), 0));
        Ensure_Seccomp(seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(read), 0));
        Ensure_Seccomp(seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(write), 0));
        Ensure_Seccomp(seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(mmap), 0));
        Ensure_Seccomp(seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(munmap), 0));
        Ensure_Seccomp(seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(mprotect), 0));
        Ensure_Seccomp(seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(rt_sigprocmask), 0));
        Ensure_Seccomp(seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(getpid), 0));
        Ensure_Seccomp(seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(gettid), 0));
        Ensure_Seccomp(seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(tgkill), 0));
        Ensure_Seccomp(seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(exit_group), 0));
        Ensure_Seccomp(seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(lseek), 0));
        Ensure_Seccomp(seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(uname), 0));
        Ensure_Seccomp(seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(arch_prctl), 0));
        Ensure_Seccomp(seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(brk), 0));

        Ensure_Seccomp(seccomp_rule_add(ctx, SCMP_ACT_TRACE(Seccomp_ExecvpSyscall), SCMP_SYS(execve), 0));

        Ensure_Seccomp(seccomp_rule_add(ctx, SCMP_ACT_ERRNO(1), SCMP_SYS(access), 0));
        Ensure_Seccomp(seccomp_rule_add(ctx, SCMP_ACT_ERRNO(1), SCMP_SYS(readlink), 0));
        Ensure_Seccomp(seccomp_rule_add(ctx, SCMP_ACT_ERRNO(1), SCMP_SYS(fstat), 0));

        Ensure_Seccomp(seccomp_load(ctx));
    }
    catch (runtime_error &rc) {
        if (ctx != nullptr)
            seccomp_release(ctx);
        throw;
    }
}

void RedirectIO(const string &input = "", const string &output = "") {
    if (input != "") {
        int inputfd = Ensure(open(input.c_str(), O_RDONLY));
        Ensure(dup2(inputfd, STDIN_FILENO));
    }

    if (output != "") {
        int outputfd = Ensure(
                open(output.c_str(), O_WRONLY | O_TRUNC | O_CREAT, S_IWUSR | S_IRUSR | S_IRGRP | S_IWGRP));
        Ensure(dup2(outputfd, STDOUT_FILENO));
    }
}

void FindAndSetUID(const string &userName) {
    passwd *pw;
    pw = CheckNull(getpwnam(userName.c_str()));
    Ensure(setuid(pw->pw_uid));
}

void SetResourceLimit(int cpu, int mem) {
    rlimit rl;

    if (cpu != -1) {
        // We can capture SIGXCPU
        rl.rlim_cur = (cpu + 999) / 1000;
        rl.rlim_max = (cpu + 999) / 1000 + 1;
        Ensure(setrlimit(RLIMIT_CPU, &rl));
    }

    if (mem != -1) {
        rl.rlim_cur = rl.rlim_max = mem * 1024 * 2;
        Ensure(setrlimit(RLIMIT_AS, &rl));
    }
}

bool ChildProcess(const string &appName,
                  const string &workDir,
                  int cpuLimit = -1,
                  int memLimit = -1,
                  const string &input = "",
                  const string &output = "",
                  const string &userName = "",
                  bool doChroot = false) {
    try {
        if (doChroot)
            Ensure(chroot(workDir.c_str()));
        else
            Ensure(chdir(workDir.c_str()));

        RedirectIO(input, output);

        SetResourceLimit(cpuLimit, memLimit);

        if (userName != "")
            FindAndSetUID(userName);

        ptrace_e(PTRACE_TRACEME, 0, NULL, NULL);
        Ensure0(raise(SIGSTOP));

        InitalizeSeccomp();
        char *margs[] = {nullptr};
        Log(format("Child: start executing"), Debug);
        Ensure(execvp(("./" + appName).c_str(), margs));
    }
    catch (std::runtime_error &err) {
        Log(format("Child: error: %1%") % err.what(), Error);
    }
}

RunResult TraceChild(int childpid) {
    int status;
    bool codeRun = false;
    RunResult runResult;
    enum {
        Running, Kill, SelfExited
    } childStatus;

    try {
        // Waiting for child to execute SIGSTOP
        Log(format("Parent: Waiting for child"), Debug);
        Ensure(waitpid(childpid, &status, 0));

        if (!((WIFSTOPPED(status)) && ((WSTOPSIG(status)) == SIGSTOP))) {
            throw std::runtime_error("Waiting for child failed: child did not raise SIGSTOP.");
        }

        Log(format("Parent: Setting ptrace options"), Debug);
        // Set ptrace options. For explanations for the options, see ptrace(2)
        ptrace_e(PTRACE_SETOPTIONS, childpid, 0, PTRACE_O_TRACESYSGOOD | PTRACE_O_TRACESECCOMP | PTRACE_O_TRACEEXEC);
        childStatus = Running;
        int code = 0;

        while (childStatus == Running) {
            // Resume child execution here
            ptrace_e(PTRACE_CONT, childpid, 0, code);

            code = 0;
            Ensure(waitpid(childpid, &status, 0));
            Log(format("Child signal received: %1%") % status, Debug);

            if (WIFSTOPPED(status)) {
                Log(format("Child stopped. STOPSIG is %1%") % (WSTOPSIG(status)), Debug);
                if (status >> 8 == (SIGTRAP | (PTRACE_EVENT_SECCOMP << 8))) {
                    unsigned long x;
                    ptrace_e(PTRACE_GETEVENTMSG, childpid, NULL, &x);
                    // If the syscall is not in the whitelist
                    if ((x == Seccomp_DisabledSyscall) ||
                        // Calling execvp in sandboxed app is also invalid
                        (codeRun && x == Seccomp_ExecvpSyscall)) {
                        int scno = ptrace(PTRACE_PEEKUSER, childpid, ORIG_RAX * 8, NULL);
                        Log(format("Info: app is calling forbidden syscall `%1%()`, killing") %
                            SyscallToString(scno + 1), Debug);
                        childStatus = Kill;
                        runResult.Status = BadSyscall;
                        runResult.Code = scno + 1;
                    }
                } else if (status >> 8 == (SIGTRAP | (PTRACE_EVENT_EXEC << 8))) {
                    // App starts running.
                    codeRun = true;
                } else if (WSTOPSIG(status) == SIGXCPU) {
                    // Time out
                    childStatus = Kill;
                    runResult.Status = TimeLimitExceeded;
                } else {
                    Log(format("Unknown signal received: %1%, passing it to child.") % SignalToString(WSTOPSIG(status)),
                        Debug);
                    code = WSTOPSIG(status);
                }
            } else if (WIFEXITED(status)) {
                Log(format("App exited with code %1%") % WEXITSTATUS(status), Debug);
                childStatus = SelfExited;
                runResult.Status = Exited;
            } else if (WIFSIGNALED(status)) {
                Log(format("App is signally killed, signal is %1%") % SignalToString(WTERMSIG(status)), Debug);
                childStatus = SelfExited;
                runResult.Status = Signaled;
                runResult.Code = WTERMSIG(status);
            }
        }

        if (childStatus == Kill) {
            Ensure(kill(childpid, SIGKILL));
        }
        Log(format("Child dead."), Debug);

        // Get resource usage statistics
        if (runResult.Status == Exited) {
            rusage res;
            getrusage(RUSAGE_CHILDREN, &res);

            long long execTime = res.ru_utime.tv_sec * 1000 + res.ru_utime.tv_usec / 1000;
            long long execMemory = res.ru_maxrss;
            runResult.Time = execTime;
            runResult.Memory = execMemory;
        }
    }
    catch (std::runtime_error &err) {
        Log(format("Exception occurred: %1%") % err.what(), Error);
        if (childStatus == Running || childStatus == Kill)
            kill(childpid, SIGKILL);

        runResult.Status = Failed;
    }
    return runResult;
}

/*int main()
{
    int timeLimit = 1000;
    int memoryLimit = 65536;

    int pipefd[2];
    Ensure(pipe(pipefd));

    int pid = Ensure(fork());
    if (pid == 0)
    {
        Ensure(close(pipefd[0]));

        int childpid = Ensure(fork());
        if (childpid == 0)
        {
            ChildProcess("app", ".", timeLimit, memoryLimit, "app.in", "app.out", "", false);
            exit(1);
        }
        else
        {
            // Pass <C-c> to app
            auto interrupt_signal = EnsureNot(signal(SIGINT, SIG_IGN), SIG_ERR);
            auto quit_signal = EnsureNot(signal(SIGQUIT, SIG_IGN), SIG_ERR);

            RunResult result = TraceChild(childpid);

            EnsureNot(signal(SIGINT, interrupt_signal), SIG_ERR);
            EnsureNot(signal(SIGQUIT, quit_signal), SIG_ERR);

            if (result.Status == Exited)
                if (result.Memory > memoryLimit)
                {
                    result.Status = MemoryLimitExceeded;
                }
                else if (result.Time > timeLimit)
                {
                    result.Status = TimeLimitExceeded;
                }

            Ensure(write(pipefd[1], &result, sizeof(result)));
        }
        exit(0);
    }
    else
    {
        Ensure(close(pipefd[1]));

        RunResult result;
        int status;
        Ensure(waitpid(pid, &status, 0));
        if (!WIFEXITED(status))
        {
            throw runtime_error("Child did not exit cleanly.");
        }

        Ensure(read(pipefd[0], &result, sizeof(result)));
        Ensure(close(pipefd[0]));

        if (result.Status == Exited)
        {
            Log(format("App exited. Memory: %1%, Time: %2%") % result.Memory % result.Time, Info);
        }
        else if (result.Status == Signaled)
        {
            Log(format("App signaled (%1%).") % SignalToString(result.Code), Info);
        }
        else if (result.Status == BadSyscall)
        {
            Log(format("App called forbidden syscall `%1%()`.") % SyscallToString(result.Code), Info);
        }
    }
}
*/

ExecutionResult
RunSandbox(path tempDirectory, string targetName, string inputFileName, string outputFileName, int timeLimit,
           int memoryLimit) {
    ExecutionResult exeResult;

    int pipefd[2];
    Ensure(pipe(pipefd));

    int pid = Ensure(fork());
    if (pid == 0) {
        Ensure(close(pipefd[0]));

        int childpid = Ensure(fork());
        if (childpid == 0) {
            ChildProcess(targetName, tempDirectory.string(), timeLimit, memoryLimit, inputFileName, outputFileName, "",
                         false);
            exit(1);
        } else {
            // Pass <C-c> to app
            auto interrupt_signal = EnsureNot(signal(SIGINT, SIG_IGN), SIG_ERR);
            auto quit_signal = EnsureNot(signal(SIGQUIT, SIG_IGN), SIG_ERR);

            // No strings can be included in it since it will be passed through a pipe.
            RunResult result = TraceChild(childpid);

            EnsureNot(signal(SIGINT, interrupt_signal), SIG_ERR);
            EnsureNot(signal(SIGQUIT, quit_signal), SIG_ERR);

            if (result.Status == Exited)
                if (result.Memory > memoryLimit) {
                    result.Status = MemoryLimitExceeded;
                } else if (result.Time > timeLimit) {
                    result.Status = TimeLimitExceeded;
                }

            Ensure(write(pipefd[1], &result, sizeof(result)));
        }
        exit(0);
    } else {
        Ensure(close(pipefd[1]));

        RunResult result;
        int status;
        Ensure(waitpid(pid, &status, 0));
        if (!WIFEXITED(status)) {
            throw runtime_error("Child did not exit cleanly.");
        }

        Ensure(read(pipefd[0], &result, sizeof(result)));
        Ensure(close(pipefd[0]));

        exeResult.Status = result.Status;

        if (result.Status == Exited) {
            exeResult.Memory = result.Memory;
            exeResult.Time = result.Time;
        } else if (result.Status == Signaled) {
            exeResult.Message = str(
                    format("Your program received %1%. Please refer to https://goo.gl/9opiOd for more details on signals.") %
                    SignalToString(result.Code));
            Log(format("App signaled (%1%).") % SignalToString(result.Code), Info);
        } else if (result.Status == BadSyscall) {
            switch (result.Code) {
                case SCMP_SYS(open):
                case SCMP_SYS(close):
                    exeResult.Message = "You are calling `open()` or `close()`. Please remove any code related to file operation in your program.";
                    break;
                default:
                    exeResult.Message = str(
                            format("Your program has called `%1%()`. If your program contains no malicious code, please contact the administrator.") %
                            SyscallToString(result.Code));
                    break;
            }
            Log(format("App called forbidden syscall `%1%()`.") % SyscallToString(result.Code), Info);
        }
    }
    return exeResult;
}
