#pragma once

#include <string>
#include <boost/filesystem.hpp>

enum RunStatus {
    // App exited normally.
            Exited,
    // App is kill by some signal.
            Signaled,
    // App is killed due to some bad syscalls.
            BadSyscall,
    TimeLimitExceeded,
    MemoryLimitExceeded,
    // Exceptions occurred while running child.
            Failed
};

struct ExecutionResult {
    RunStatus Status;
    std::string Message;
    int Time;
    int Memory;
};

ExecutionResult RunSandbox(boost::filesystem::path tempDirectory, std::string targetName, std::string inputFileName,
                           std::string outputFileName, int timeLimit, int memoryLimit);
