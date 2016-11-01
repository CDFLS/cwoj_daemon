//
// Created by zhangyutong926 on 10/24/16.
//

#include <cstdarg>
#include <sstream>
#include <chrono>

#include "cwojlog.h"

using namespace boost::filesystem;
using std::cout;
using std::string;
using std::ostringstream;
using namespace std::chrono;

CwojLogger *DefaultLogger = nullptr;

void InitDefaultLogger(DaemonConfiguration conf) {
    DefaultLogger = new CwojLogger(
            conf.NormalLogFile,
            conf.ExceptionLogFile,
            conf.DebugMode
    );
    DefaultLogger->Log(CwojLogLevel::IMPORTANT_INFO, "Default logging is working now.");
}

CwojLogger::CwojLogger(path normalLogFile,
                       path exceptionLogFile,
                       bool isDebugMode = false):
        _normalLogFile(normalLogFile),
        _exceptionLogFile(exceptionLogFile),
        _debugMode(isDebugMode) {
    OpenStreams();
}

CwojLogger::~CwojLogger() {
    FlushStreams();
    CloseStreams();
}

void CwojLogger::Log(CwojLogLevel level, std::string content) {
    string format = FormatLog(level, content);
    switch (level) {
        case DEBUG:
            if (_debugMode) {
                cout << format;
            }
            break;
        case INFO:
            cout << format;
            break;
        case IMPORTANT_INFO:
            cout << format;
            *_normalStream << format;
            break;
        case WARNING:
            cout << format;
            *_normalStream << format;
            *_exceptionStream << format;
            break;
        case FETAL_ERROR:
            cout << format;
            *_normalStream << format;
            *_exceptionStream << format;
            Terminate();
            break;
    }
}

void CwojLogger::OpenStreams() {
    _normalStream = new ofstream(_normalLogFile);
    _exceptionStream = new ofstream(_exceptionLogFile);
}

void CwojLogger::CloseStreams() {
    _normalStream->close();
    _exceptionStream->close();
}

//FIXME Fool code
string CwojLogger::FormatLog(CwojLogLevel level, string content) {
    string s;
    switch (level) {
        case DEBUG:
            s = "DEBUG";
            break;
        case INFO:
            s = "INFO";
            break;
        case IMPORTANT_INFO:
            s = "IMPORTANT INFO";
            break;
        case WARNING:
            s = "WARNING";
            break;
        case FETAL_ERROR:
            s = "FETAL ERROR";
            break;
        default:
            s = "UNKNOWN";
    }
    return "[" LOG_PREFFIX "][" +
            string(GetTimeStamp()) +
            "][" + s + "]  " + content;
}

void CwojLogger::Terminate() {
    exit(1);
}

string CwojLogger::GetTimeStamp() {
    time_t now_time = time(NULL);
    char time_str[24];
    std::strftime(time_str, 24, "%F %T", std::localtime(&now_time));
    return string(time_str);
}

void CwojLogger::FlushStreams() {
    _normalStream->flush();
    _exceptionStream->flush();
}
