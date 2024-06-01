@echo off
g++ -std=c++23 -O0 -g main.cpp -o main.exe -l ws2_32 -l winmm && ( echo Build Success ) || ( echo Build Failed )
@pause >nul