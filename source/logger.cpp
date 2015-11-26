//
// Created by kamlesh on 30/9/15.
//

#include "logger.h"

using namespace pigeon;

logger::logger() {
    if (!lf.is_open()) {
        lf.open(settings::log_location, ios::app);
    }
}

logger::logger(const logger &lgr){

}

logger::~logger() {

}

void logger::write(LogType type, Severity severity, string message) {

    lf << get_log_type(type) << ": " << get_severity(severity) << ": " << now() << " : " << message << endl;
    lf.flush();

}

void logger::write(LogType type, string message) {

    lf << get_log_type(type) << ": " << now() << " : " << message << endl;
    lf.flush();

}

void logger::close() {
    if (lf.is_open()) {
        lf.close();
    }
}

std::shared_ptr<logger> logger::instance = nullptr;

std::mutex logger::_mtx;

std::shared_ptr<logger>&logger::get()
{
    static std::shared_ptr<logger> tmp = instance;

    if (!tmp)
    {
        std::lock_guard<std::mutex> lock(_mtx);
        if (!tmp)
        {
            instance.reset(new logger);
            tmp = instance;
        }
    }

    return tmp;
}

