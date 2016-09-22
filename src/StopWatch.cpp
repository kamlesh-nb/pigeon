#include "StopWatch.h"


using namespace pigeon;


StopWatch::StopWatch() {
    begin = std::chrono::high_resolution_clock::now();
    end = std::chrono::high_resolution_clock::now();
}

StopWatch::~StopWatch() { }

void StopWatch::Start() {
    begin = std::chrono::high_resolution_clock::now();
}

void StopWatch::Stop() {
    end = std::chrono::high_resolution_clock::now();
}

int64_t StopWatch::ElapsedMiliSeconds() {
    return std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count();
}

int64_t StopWatch::ElapsedMicroSeconds() {
    return std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count();
}

int64_t StopWatch::ElapsedNanoSeconds() {
    return std::chrono::duration_cast<std::chrono::nanoseconds>(end - begin).count();
}

double StopWatch::ElapsedSeconds() {
    auto diff = end - begin;
    return std::chrono::duration<double>(diff).count();
}
