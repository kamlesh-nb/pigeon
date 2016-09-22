#ifndef STOP_WATCH_H
#define STOP_WATCH_H

#include <stdint.h>
#include <random>
#include <time.h>
#include <string.h>
#include <chrono>
#include <cstdint>

namespace pigeon {

    class StopWatch {
    public:
        StopWatch();
        ~StopWatch();
        void Start();
        void Stop();
        int64_t ElapsedMiliSeconds();
        int64_t ElapsedMicroSeconds();
        int64_t ElapsedNanoSeconds();
        double ElapsedSeconds();
    private:
        std::chrono::high_resolution_clock::time_point begin;
        std::chrono::high_resolution_clock::time_point end;
    };

}


#endif //STOP_WATCH_H