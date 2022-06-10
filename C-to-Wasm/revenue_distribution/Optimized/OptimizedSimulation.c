#include <time.h>
#include "DistributionContract.h"
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#define NB_USERS 1000000
#define ITER 100

long benchmark(void(*fun)(void*), void* data) {
    long averageTime = 0;
    struct timespec begin, end; 
    // Setup
    for(size_t i = 0; i < ITER; ++i) {
        clock_gettime(CLOCK_MONOTONIC, &begin);
        // Start benchmarked code
        fun(data);
        // Cleanup
        clock_gettime(CLOCK_MONOTONIC, &end);
        if(end.tv_nsec <= begin.tv_nsec) continue;
        averageTime += (end.tv_nsec - begin.tv_nsec);
    }
    return averageTime / ITER;
}

void distributeRevenueBenchmark(void* data) {
    DistributionContract* contract = data;
    addRevenue(contract, NB_USERS);
}

int main() {
    // Setup
    DistributionContract* contract = constructContract();
    char* addresses[NB_USERS] = {0};
    for(size_t i = 0 ; i < NB_USERS ; ++i) {
        addresses[i] = calloc(1, 8 * sizeof(char));
        sprintf(addresses[i], "%zu", i + 1);
        changeShare(contract, addresses[i], i + 1);
    }
    // Warmup
    benchmark(distributeRevenueBenchmark, contract);
    // Result
    printf("TEMPS: %lu\n", benchmark(distributeRevenueBenchmark, contract));
    // Cleanup
    for(size_t i = 0 ; i < NB_USERS ; ++i) free(addresses[i]);
    destroyContract(contract);
}