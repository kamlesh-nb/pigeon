#include "stop_watch.h"


using namespace pigeon;


stop_watch::stop_watch(){
	begin = std::chrono::steady_clock::now();
	end = std::chrono::steady_clock::now();
}

stop_watch::~stop_watch() {}

void stop_watch::start() {
	begin = std::chrono::high_resolution_clock::now();
}

void stop_watch::stop() {
	end = std::chrono::high_resolution_clock::now();
}

int64_t stop_watch::elapsed_miliseconds() {
	return std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count();
}

int64_t stop_watch::elapsed_microseconds() {
	return std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count();
}

int64_t stop_watch::elapsed_nanoseconds() {
	return std::chrono::duration_cast<std::chrono::nanoseconds>(end - begin).count();
}

double stop_watch::elapsed_seconds() {
	auto diff = end - begin;
	return std::chrono::duration<double>(diff).count();
}
