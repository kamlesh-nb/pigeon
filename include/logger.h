//
// Created by kamlesh on 30/9/15.
//

#ifndef PIGEON_LOGGER_H
#define PIGEON_LOGGER_H

#include <mutex>
#include <fstream>
#include <string.h>
#include <memory>
#include "http_util.h"

using namespace std;


namespace pigeon {


    class logger {

        static std::mutex _mtx;
        static std::shared_ptr<logger> instance;
        ofstream lf;

        logger();

    public:


        logger(const logger &lgr);
        logger& operator = (logger const &);
        ~logger();

        void write(LogType, Severity, string);

        void write(LogType, string);

        void close();

        static std::shared_ptr<logger> &get();


    };
}
#endif //PIGEON_LOGGER_H
