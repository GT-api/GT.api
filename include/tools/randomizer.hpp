#pragma once
#include <random>

extern thread_local std::mt19937_64 engine;

int randomizer(int min, int max);
float randomizer(float min, float max);
