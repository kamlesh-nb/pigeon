//
// Created by kamlesh on 30/9/15.
//

#include "Settings.h"
#include "Logger.h"

using namespace pigeon;

Logger::Logger() {
    if (!lf.is_open()) {
        lf.open(Settings::LogLocation, ios::app);
    }
}

Logger::~Logger() {

}

Logger::Logger(const Logger& lgr) {
    instance = lgr.instance;
}

Logger& Logger::operator = (Logger const &lgr) {
    if (this != &lgr) {
       instance = lgr.instance;
    }
    return *this;
}

void Logger::Write(LogType type, Severity severity, string message) {

    lf << GetLogType(type) << ": " << GetSeverity(severity) << ": " << Now() << " : " << message << endl;
    lf.flush();

}

void Logger::Write(LogType type, string message) {

    lf << GetLogType(type) << ": " << Now() << " : " << message << endl;
    lf.flush();

}

void Logger::Close() {
    if (lf.is_open()) {
        lf.close();
    }
}

std::shared_ptr<Logger> Logger::instance = nullptr;

std::mutex Logger::_mtx;

std::shared_ptr<Logger> &Logger::Get() {
    static std::shared_ptr<Logger> tmp = instance;

    if (!tmp) {
        std::lock_guard<std::mutex> lock(_mtx);
        if (!tmp) {
            instance.reset(new Logger);
            tmp = instance;
        }
    }

    return tmp;
}

