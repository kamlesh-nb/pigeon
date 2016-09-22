//
// Created by kamlesh on 30/9/15.
//

#ifndef PIGEON_LOGGER_H
#define PIGEON_LOGGER_H

#include <mutex>
#include <fstream>
#include <string.h>
#include <memory>
#include "HttpUtil.h"

using namespace std;
using namespace pigeon::net;

namespace pigeon {

    class Logger {
    private:
        static std::mutex _mtx;
        static std::shared_ptr<Logger> instance;
        ofstream lf;
        Logger();
    public:
        Logger(const Logger &lgr);
        Logger& operator = (Logger const &);
        ~Logger();
        void Write(LogType, Severity, string);
        void Write(LogType, string);
        void Close();
        static std::shared_ptr<Logger> &Get();

    };
}
#endif //PIGEON_LOGGER_H
