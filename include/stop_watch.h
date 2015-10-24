#ifndef STOP_WATCH_H
#define STOP_WATCH_H

#include <iostream>
#include <vector>
#include <string>
#include <stdint.h>
#include <random>
#include <time.h>
#include <string.h>
#include <chrono>
#include <cstdint>

namespace pigeon {

    namespace util {

        class stop_watch {
        public:
            stop_watch();

            ~stop_watch();

            void start();

            void stop();

            int64_t elapsed_miliseconds();

            int64_t elapsed_microseconds();

            int64_t elapsed_nanoseconds();

            double elapsed_seconds();

        private:
            std::chrono::steady_clock::time_point begin;
            std::chrono::steady_clock::time_point end;


        };

    }

}


#endif //STOP_WATCH_H