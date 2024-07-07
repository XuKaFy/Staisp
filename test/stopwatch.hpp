#pragma once


#include <chrono>
class Stopwatch {
private:
    std::chrono::time_point<std::chrono::steady_clock> start_time;

public:

    void reset() {
        start_time = std::chrono::steady_clock::now();
    }

    double stop() {
        auto end_time = std::chrono::steady_clock::now();
        std::chrono::duration<double> elapsed_time = end_time - start_time;
        return elapsed_time.count();
    }
};
