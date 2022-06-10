#pragma once

#include <gsl/gsl_matrix.h>

#define IS_VECTOR(m) \
    if((m)->size1 != 1 && (m)->size2 != 1) \
        return NULL \

#define REQUIRE_NON_NULL(e) \
    if(e == NULL) \
        return NULL \

gsl_matrix* normalize(const gsl_matrix* m);
void destroyMatricesArray(gsl_matrix** array, size_t nbElements);
gsl_matrix* nn(const gsl_matrix* x, const gsl_matrix** allW, size_t nbW);
gsl_matrix** train(gsl_matrix* trainInput, gsl_matrix* trainOutput, int* dimensions, size_t nbDimensions, int numEpoch, double learningRate);