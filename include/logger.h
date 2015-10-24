//
// Created by kamlesh on 30/9/15.
//

#ifndef PIGEONWEB_LOGGER_H
#define PIGEONWEB_LOGGER_H

#include <mutex>
#include <fstream>
#include <string.h>
#include <memory>
#include "settings.h"
#include "http_util.h"

using namespace std;
using namespace http_util;

namespace pigeon {


    class logger {

        static std::mutex _mtx;
        static std::shared_ptr<logger> instance;
        ofstream lf;
        logger(app*);

    public:


        logger(const logger &lgr);
        ~logger();
        void write(http_util::LogType, http_util::Severity, string);
        void write(http_util::LogType, string);
        void close();
        static std::shared_ptr<logger>& get(app*);


    };
}
#endif //PIGEONWEB_LOGGER_H
