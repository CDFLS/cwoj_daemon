//
// Created by zhangyutong926 on 10/24/16.
//

#ifndef CWOJ_DAEMON_CWOJLOG_H
#define CWOJ_DAEMON_CWOJLOG_H

#include <iostream>
#include <string>
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
#include "../config/config_item.h"

#define LOG_PREFFIX "CWOJ_DAEMON"

enum CwojLogLevel {
    DEBUG, // Which would not be recorded with any method unless the program is running under debug-mode, if so, debug info is CLI only
    INFO, // Which will be recorded only with CLI and no file recording
    IMPORTANT_INFO, // Which will be recorded with both CLI and normal-level log file
    WARNING, // Which will be recorded with CLI, normal-level log file and exception-level log file
    FETAL_ERROR // Which will try to storage temporary data and terminate the program immediately, and the log will be recorded with CLI, normal-level log file and exception-level log file
};

class CwojLogger {
public:
    CwojLogger(boost::filesystem::path,
               boost::filesystem::path,
               bool);

    ~CwojLogger();

    void Log(CwojLogLevel, std::string);

private:
    boost::filesystem::path _normalLogFile, _exceptionLogFile;
    boost::filesystem::ofstream *_normalStream, *_exceptionStream;
    bool _debugMode;

    void OpenStreams();

    void FlushStreams();

    void CloseStreams();

    std::string GetTimeStamp();

    std::string FormatLog(CwojLogLevel, std::string);

    void Terminate();
};

extern CwojLogger *DefaultLogger;

void InitDefaultLogger(DaemonConfiguration conf);

#endif //CWOJ_DAEMON_CWOJLOG_H
