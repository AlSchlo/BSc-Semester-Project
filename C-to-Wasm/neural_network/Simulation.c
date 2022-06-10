#include <time.h>
#include "ClassificationContract.h"
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#define NB_TRAIN_SAMPLES 100
#define NB_FEATURES 5
#define NB_TEST_SAMPLES 2
#define NB_CLASSES 2
#define NB_EPOCH 1
#define LEARNING_RATE 0.01
#define NB_LAYERS 1
#define LAYER_SIZE 3

#define ITER 100

static double* generateData(int rows, int cols) {
    double* data = calloc(rows * cols, sizeof(double));
    if(!data) return NULL;

    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            data[(i * cols) + j] = ((i * cols) + j) % 1000;
        }
    }

    return data;
}

static int* generateLayers() {
    int* layers = calloc(NB_LAYERS, sizeof(int));
    if(!layers) return NULL;

    for (int i = 0; i < NB_LAYERS; i++) {
        layers[i] = LAYER_SIZE;
    }

    return layers;
}

long benchmark(void(*fun)(void*), void* data) {
    struct timespec begin, end; 
    // Setup
    clock_gettime(CLOCK_MONOTONIC, &begin);
    // Start benchmarked code
    fun(data); 
    // Cleanup
    clock_gettime(CLOCK_MONOTONIC, &end);
    if(end.tv_nsec <= begin.tv_nsec) return 0;
    return (end.tv_nsec - begin.tv_nsec);
}

void normalizeBenchmark(void* data) {
    ClassificationContract* contract = data;
    normalizeContract(contract);
}

void trainBenchmark(void* data) {
    ClassificationContract* contract = data;
    trainContract(contract, NB_EPOCH, LEARNING_RATE);
}

void testBenchmark(void* data) {
    ClassificationContract* contract = data;
    testContract(contract);
}

int main() {
    long totalNorm = 0;
    long totalTrain = 0;
    long totalTest = 0;

    for(int i = 0; i < ITER; i++) {
        // Setup
        double* X = generateData(NB_TRAIN_SAMPLES, NB_FEATURES);
        double* Y = generateData(NB_TRAIN_SAMPLES, NB_CLASSES);
        double* tX = generateData(NB_TEST_SAMPLES, NB_FEATURES);
        double* tY = generateData(NB_TEST_SAMPLES, NB_CLASSES);
        int* hiddenLayers = generateLayers();

        ClassificationContract* contract = constructContract(X, Y, tX, tY, hiddenLayers, NB_TRAIN_SAMPLES, 
                                    NB_FEATURES, NB_CLASSES, NB_TEST_SAMPLES, NB_LAYERS);
        
        free(X); free(Y); free(tX); free(tY); free(hiddenLayers);

        // Result
        size_t resultNorm = benchmark(normalizeBenchmark, contract);
        size_t resultTrain = benchmark(trainBenchmark, contract);
        size_t resultTest = benchmark(testBenchmark, contract);
    
        if(!resultNorm || !resultTrain || !resultTest) i--;

        totalNorm += resultNorm;
        totalTrain += resultTrain;
        totalTest += resultTest;

        // Cleanup
        destroyContract(contract);
    }

    printf("NORM: %lu\n", totalNorm / ITER);
    printf("TRAIN: %lu\n", totalTrain / ITER);
    printf("TEST: %lu\n", totalTest / ITER);
}