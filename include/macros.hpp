#pragma once
#include <chrono>

#ifdef DEBUG_TIME
    #define TIME_START auto start = std::chrono::high_resolution_clock::now()
    #define TIME_END(str) \
        auto end = std::chrono::high_resolution_clock::now(); \
        printf("%s: %.2f ms\n", str, std::chrono::duration<double, std::milli>{end - start}.count())
#else
    #define TIME_START
    #define TIME_END(str)
#endif
