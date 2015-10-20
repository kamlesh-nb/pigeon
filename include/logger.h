//
// Created by kamlesh on 30/9/15.
//

#ifndef PIGEONWEB_LOGGER_H
#define PIGEONWEB_LOGGER_H

#include <mutex>
#include <fstream>
#include <string.h>
#include <memory>
#include "config.h"
#include "app_constants.h"

using namespace std;

namespace pigeon {
    class logger {

        static std::mutex _mtx;
        static std::shared_ptr<logger> instance;
        ofstream lf;

    public:

        logger();
        logger(const logger &lgr);
        ~logger();
        void write(LogType, Severity, string);
        void write(LogType, string);
        void close();
        static std::shared_ptr<logger>& get();

    };
}
#endif //PIGEONWEB_LOGGER_H
