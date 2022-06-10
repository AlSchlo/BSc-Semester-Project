#pragma once

#include <stdint.h>

typedef struct {
    int64_t seed;
} Random;

Random initSeed(int64_t seed);
double nextDouble(Random* r);
double nextDoubleRange(Random* r, double low, double high);