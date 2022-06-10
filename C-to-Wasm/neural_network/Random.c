#include "Random.h"
#include <stdint.h>

const int64_t multiplier = 0x5DEECE66D;
const int64_t addend = 0xB;
const int64_t mask = ((int64_t)(1) << 48) - 1;
const int64_t denom = (int64_t)(1) << 53;

static int32_t nextBits(Random* r, int bits) {
    int64_t nextSeed = (r->seed * multiplier + addend) & mask;
    r->seed = nextSeed;
    int64_t sign = nextSeed >= 0 ? 1 : 0;
    int64_t arithmeticMask = ~((sign << (16 + bits)) - 1);
    r->seed = nextSeed;
    return (int32_t)(arithmeticMask | (nextSeed >> (48 - bits)));
}

Random initSeed(int64_t seed) {
    Random r = { (seed ^ multiplier) & mask };
    return r;
}

double nextDoubleRange(Random* r, double low, double high) {
    double d = nextDouble(r);
    return ((high - low) * d) + low;
}

double nextDouble(Random* r) {
    int64_t v1 = ((int64_t)(nextBits(r, 26)) << 27);
    int64_t v2 = (int64_t)(nextBits(r, 27));
    return (double)(v1 + v2) / (double) denom;
}