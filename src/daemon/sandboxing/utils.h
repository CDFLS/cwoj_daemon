#pragma once

#include <string>
#include <system_error>

std::string SyscallToString(int syscall);

std::string SignalToString(int signal);

void Ensure_Seccomp(int XX);

int Ensure(int XX);

void Ensure0(int XX);

template<typename T>
T EnsureNot(T ret, T err) {
    if (ret == err) {
        int errcode = errno;
        throw std::system_error(errcode, std::system_category());
    }
}

template<typename ...Args>
int ptrace_e(Args ...args) {
    return Ensure(ptrace(args...));
}

template<typename T>
T CheckNull(T val) {
    if (val == nullptr) {
        throw std::system_error(errno, std::system_category());
    }
    return val;
}
