#include <chrono>

std::chrono::time_point<std::chrono::system_clock> start;

bool clock_start()
{
    start = std::chrono::system_clock::now();
}

float clock_end()
{
    auto end = std::chrono::system_clock::now();
    return std::chrono::duration_cast<std::chrono::milliseconds>(end-start).count();
}

