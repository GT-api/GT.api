
#define debug_time // @note display how long tasks complete (beta)

#ifdef debug_time
#define time_start auto start = std::chrono::high_resolution_clock::now();
#else
#define time_start
#endif

#ifdef debug_time
#define time_end(exe) \
    auto end = std::chrono::high_resolution_clock::now(); \
    printf("%s: %.2f ms\n", exe, std::chrono::duration<double, std::milli>{end - start}.count());
#else
#define time_start(exe)
#endif