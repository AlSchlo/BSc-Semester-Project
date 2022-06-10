#include <gsl/gsl_matrix.h>
#include "MatrixNNUtils.h"
#include "ClassificationContract.h"

static gsl_matrix* fromArray(double* data, size_t rows, size_t cols) {
    gsl_matrix* matrix = gsl_matrix_calloc(rows, cols);
    if(!matrix) return NULL;

    for(size_t i = 0; i < rows; i++) {
        for(size_t j = 0; j < cols; j++) {
            gsl_matrix_set(matrix, i, j, data[i * cols + j]);
        }
    }

    return matrix;
}

ClassificationContract* constructContract(double* X, double* Y, double* tX, double* tY, int* hiddenLayers, 
    size_t N, size_t M, size_t C, size_t T, size_t nbLayers) {
        REQUIRE_NON_NULL(X);
        REQUIRE_NON_NULL(Y);
        REQUIRE_NON_NULL(tX);
        REQUIRE_NON_NULL(tY);
        REQUIRE_NON_NULL(hiddenLayers);

        ClassificationContract* contract = calloc(1, sizeof(ClassificationContract));
        if(!contract) return NULL;

        gsl_matrix* mX = fromArray(X, N, M);
        gsl_matrix* mY = fromArray(Y, N, C);
        gsl_matrix* mtX = fromArray(tX, T, M);
        gsl_matrix* mtY = fromArray(tY, T, C);
        
        if(!mX || !mY || !mtX || ! mtY) {
            if(!mX) gsl_matrix_free(mX);
            if(!mY) gsl_matrix_free(mY);
            if(!mtX) gsl_matrix_free(mtX);
            if(!mtY) gsl_matrix_free(mtY);
            free(contract);
            return NULL;
        }

        contract->trainInput = mX;
        contract->trainOutput = mY;
        contract->testInput = mtX;
        contract->testOutput = mtY;

        contract->dimensions = calloc(nbLayers + 2, sizeof(int));
        if(!contract->dimensions) {
            gsl_matrix_free(mX);
            gsl_matrix_free(mY);
            gsl_matrix_free(mtX);
            gsl_matrix_free(mtY);
            free(contract);
            return NULL;
        }
        
        contract->dimensions[0] = M;
        for(size_t i = 0; i < nbLayers; i++) {
            contract->dimensions[i + 1] = hiddenLayers[i];
        }
        contract->dimensions[nbLayers + 1] = C;

        contract->nbDimensions = nbLayers + 2;

        contract->allW = NULL;

        return contract;
    }

void destroyContract(ClassificationContract* contract) {
    if(!contract) return;

    destroyMatricesArray(contract->allW, contract->nbDimensions - 1);
    
    gsl_matrix_free(contract->trainInput);
    gsl_matrix_free(contract->trainOutput);
    gsl_matrix_free(contract->testInput);
    gsl_matrix_free(contract->testOutput);

    free(contract->dimensions);
    
    free(contract);
}

void normalizeContract(ClassificationContract* contract) {
    if(!contract) return;

    gsl_matrix* normX = normalize(contract->trainInput);
    gsl_matrix* normtX = normalize(contract->testInput);

    gsl_matrix_free(contract->trainInput);
    gsl_matrix_free(contract->testInput);

    contract->trainInput = normX;
    contract->testInput = normtX;
}

void trainContract(ClassificationContract* contract, int numEpoch, double learningRate) {
    if(!contract) return;

    contract->allW = train(contract->trainInput, contract->trainOutput, contract->dimensions, 
                            contract->nbDimensions, numEpoch, learningRate);

}

double testContract(ClassificationContract* contract) {
    // Check if the model has been trained
    if(!contract || !contract->allW) return -1;
    
    int nbSame = 0;
    int totalSamples = contract->testInput->size1;
    for(int i = 0; i < totalSamples; i++) {
        gsl_matrix inRow = gsl_matrix_const_submatrix(contract->testInput, i, 0, 1, contract->testInput->size2).matrix;
        gsl_matrix* tInRow = gsl_matrix_calloc(inRow.size2, inRow.size1);

        if(!tInRow || gsl_matrix_transpose_memcpy(tInRow, &inRow)) {
            if(tInRow) gsl_matrix_free(tInRow);
            return -1;
        }

        gsl_matrix* result = nn(tInRow, (const gsl_matrix**) contract->allW, contract->nbDimensions - 1);
        if(!result) {
            gsl_matrix_free(tInRow);
            return -1;
        }

        gsl_vector_const_view vPred = result->size1 == 1 ? gsl_matrix_const_row(result, 0) : gsl_matrix_const_column(result, 0);
        size_t prediction = gsl_vector_max_index(&vPred.vector);

        gsl_matrix outRow = gsl_matrix_const_submatrix(contract->testOutput, i, 0, 1, contract->testOutput->size2).matrix;
        gsl_vector_const_view vAct = outRow.size1 == 1 ? gsl_matrix_const_row(&outRow, 0) : gsl_matrix_const_column(&outRow, 0);
        size_t expected = gsl_vector_max_index(&vAct.vector);

        if(prediction == expected) nbSame++;
        gsl_matrix_free(tInRow);
        gsl_matrix_free(result);
    }

    return (double) nbSame / (double) totalSamples;
}