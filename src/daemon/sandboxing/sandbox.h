#pragma once

#include <string>
#include <boost/filesystem.hpp>

enum RunStatus {
    EXITED, // App exited normally.
    SIGNALED, // App is kill by some signal.
    BAD_SYSTEM_CALL, // App is killed due to some bad syscalls.
    TIME_LIMIT_EXCEEDED,
    MEMORY_LIMIT_EXCEEDED,
    FAILED // Exceptions occurred while running child.
};

struct ExecutionResult {
    RunStatus Status;
    std::string Message;
    int Time;
    int Memory;
};

ExecutionResult RunSandbox(boost::filesystem::path,
                           std::string,
                           std::string,
                           std::string,
                           int,
                           int);
