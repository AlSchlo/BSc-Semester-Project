#pragma once

#include <gsl/gsl_matrix.h>

typedef struct {
    gsl_matrix* trainInput;
    gsl_matrix* testInput;
    gsl_matrix* trainOutput;
    gsl_matrix* testOutput;
    int* dimensions;
    size_t nbDimensions;
    gsl_matrix** allW;
} ClassificationContract;

/**
 * @brief Constructs a classification contract
 *  
 * @param X The raw N samples of M dimensions matrix (N x M)
 * @param Y The raw N classification encoded as a probability between the C classes (N x C)
 * @param tX The raw N samples of T dimensions matrix (T x M)
 * @param tY The raw N classification encoded as a probability between the C classes (T x C)
 * @param hiddenLayers The dimensions (and number) of hidden layers
 * @param N See above
 * @param M See above
 * @param C See above
 * @param T See above
 * @param nbLayers See above
 * 
 * @return ClassificationContract*
 */
ClassificationContract* constructContract(double* X, double* Y, double* tX, double* tY, int* hiddenLayers, 
    size_t N, size_t M, size_t C, size_t T, size_t nbLayers);

/**
 * @brief Destroys a classification contract
 * 
 * @param contract The contract to be destroyed
 */
void destroyContract(ClassificationContract* contract);

/**
 * @brief Normalizes the data in the contract
 * 
 * @param contract 
 */
void normalizeContract(ClassificationContract* contract);

/**
 * @brief Trains the data with a given number of epochs and learning rate
 * 
 * @param numEpoch
 * @param learningRate
 * 
 * @param contract 
 */
void trainContract(ClassificationContract* contract, int numEpoch, double learningRate);

/**
 * @brief Tests the contract after training and returns the accuracy
 * 
 * @param contract 
 * 
 * @return accuracy
 */
double testContract(ClassificationContract* contract);