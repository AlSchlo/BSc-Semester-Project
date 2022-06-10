#include <gsl/gsl_matrix.h>
#include <gsl/gsl_blas.h>
#include <math.h>
#include "Random.h"
#include "MatrixNNUtils.h"

gsl_matrix* relu(const gsl_matrix* m) {
    REQUIRE_NON_NULL(m);
    IS_VECTOR(m);

    gsl_matrix* out = gsl_matrix_calloc(m->size1, m->size2);
    if(!out) return NULL;

    for(size_t i = 0; i < m->size1; i++) {
        for(size_t j = 0; j < m->size2; j++) {
            double old = gsl_matrix_get(m, i, j);
            gsl_matrix_set(out, i, j, fmax(old, 0));
        }
    }

    return out;
}

gsl_matrix* identity(const gsl_matrix* m) {
    REQUIRE_NON_NULL(m);
    IS_VECTOR(m);

    gsl_matrix* out = gsl_matrix_calloc(m->size1, m->size2);
    if(!out) return NULL;

    if(gsl_matrix_memcpy(out, m)) {
        gsl_matrix_free(out);
        return NULL;
    }

    return out;
}

gsl_matrix* dIdentityDx(const gsl_matrix* x) {
    REQUIRE_NON_NULL(x);
    IS_VECTOR(x);

    size_t nbElements = x->size1 * x->size2;
    gsl_matrix* out = gsl_matrix_calloc(nbElements, nbElements);
    if(!out) return NULL;

    gsl_matrix_set_identity(out);

    return out;
}

gsl_matrix* reshape(const gsl_matrix* m, size_t rows, size_t cols) {
    REQUIRE_NON_NULL(m);
    if(rows * cols != m->size1 * m->size2) return NULL;

    gsl_matrix* out = gsl_matrix_calloc(rows, cols);
    if(!out) return NULL;


    for(size_t i = 0; i < m->size1; i++) {
        for(size_t j = 0; j < m->size2; j++) {
            size_t n = i * m->size2 + j;
            size_t dI = n / cols;
            size_t dJ = n % cols;
            gsl_matrix_set(out, dI, dJ, gsl_matrix_get(m, i, j));         
        }
    }

    return out;
}

gsl_matrix* dL2Dx(const gsl_matrix* x, const gsl_matrix* y) {
    REQUIRE_NON_NULL(x);
    REQUIRE_NON_NULL(y);
    IS_VECTOR(x);
    IS_VECTOR(y);

    gsl_matrix* out = gsl_matrix_calloc(x->size1, x->size2);
    if(!out) return NULL;

    if(gsl_matrix_memcpy(out, x)) {
        gsl_matrix_free(out);
        return NULL;
    }

    if(gsl_matrix_sub(out, y) || gsl_matrix_scale(out, 2.0)) {
        gsl_matrix_free(out);
        return NULL;
    }

    gsl_matrix* reshaped = reshape(out, 1, x->size1 * x->size2);
    gsl_matrix_free(out);

    return reshaped;
}

gsl_matrix* affine(const gsl_matrix* x, const gsl_matrix* W) {
    REQUIRE_NON_NULL(x);
    REQUIRE_NON_NULL(W);
    IS_VECTOR(x);

    size_t nbElements = x->size1 * x->size2;
    gsl_matrix* extendedX = gsl_matrix_calloc(nbElements + 1, 1);
    if(!extendedX) return NULL;

    for(size_t i = 0; i < nbElements; i++) {
        gsl_vector_const_view row = gsl_matrix_const_row(x, i);
        if(gsl_matrix_set_row(extendedX, i, &row.vector)) {
            gsl_matrix_free(extendedX);
            return NULL;
        }
    }
    gsl_matrix_set(extendedX, nbElements, 0, 1.0);
    
    gsl_matrix* out = gsl_matrix_calloc(W->size1, extendedX->size2);
    if(!out) {
        gsl_matrix_free(extendedX);
        return NULL;
    }

    if(gsl_blas_dgemm(CblasNoTrans, CblasNoTrans, 1.0, W, extendedX, 0.0, out)) {
        gsl_matrix_free(extendedX);
        gsl_matrix_free(out);
        return NULL;
    }

    gsl_matrix_free(extendedX);
    return out;
}

gsl_matrix* initMatrix(size_t rows, size_t cols) {
    gsl_matrix* out = gsl_matrix_calloc(rows, cols + 1);
    if(!out) return NULL;

    Random r = initSeed(0);
    double l = sqrt(3.0 / (double) cols);

    gsl_vector* zeros = gsl_vector_calloc(rows);
    if(!zeros) {
        gsl_vector_free(zeros);
        return NULL;
    }

    for(size_t i = 0; i < rows; i++) {
        for(size_t j = 0; j < cols; j++) {
            gsl_matrix_set(out, i, j, nextDoubleRange(&r, -l, l));
        }
    }

    if(gsl_matrix_set_col(out, cols, zeros)) {
        gsl_vector_free(zeros);
        gsl_matrix_free(out);
        return NULL;
    }

    gsl_vector_free(zeros);
    return out;
}

gsl_matrix** initNetwork(const int* dimensions, size_t nbDimensions) {
    REQUIRE_NON_NULL(dimensions);
    if(nbDimensions <= 1) return NULL;

    gsl_matrix** out = calloc(nbDimensions - 1, sizeof(gsl_matrix*));
    for(size_t i = 0; i < nbDimensions - 1; i++) {
        int Wi = dimensions[i];
        int Wip1 = dimensions[i + 1];

        out[i] = initMatrix(Wip1, Wi);

        if(!out[i]) {
            for(size_t j = 0; j < i; j++) {
                gsl_matrix_free(out[j]);
            }
            free(out);
            return NULL;
        }
    }

    return out;
}

void destroyMatricesArray(gsl_matrix** array, size_t nbElements) {
    if(!array) return;
    for(size_t i = 0; i < nbElements; i++) {
        if(array[i]) gsl_matrix_free(array[i]);
    }
    free(array);
}

gsl_matrix* dReluDx(const gsl_matrix* x) {
    REQUIRE_NON_NULL(x);
    IS_VECTOR(x);

    size_t nbElements = x->size1 * x->size2;
    gsl_matrix* out = gsl_matrix_calloc(nbElements, nbElements);
    if(!out) return NULL;

    for(size_t i = 0; i < nbElements; i++) {
        double value;
        if(x->size1 > x->size2) {
            value = gsl_matrix_get(x, i, 0) > 0 ? 1 : 0;
        } else {
            value = gsl_matrix_get(x, 0, i) > 0 ? 1 : 0;
        }
        gsl_matrix_set(out, i, i, value);
    }

    return out;
}

gsl_matrix* dAffineDx(const gsl_matrix* W) {
    REQUIRE_NON_NULL(W);

    gsl_matrix* out = gsl_matrix_calloc(W->size1, W->size2 - 1);
    gsl_matrix_const_view subMatrix = gsl_matrix_const_submatrix(W, 0, 0, W->size1, W->size2 - 1);
    if(gsl_matrix_memcpy(out, &subMatrix.matrix)) {
        gsl_matrix_free(out);
        return NULL;
    }

    return out;
}

gsl_matrix* dAffineDw(const gsl_matrix* x, const gsl_matrix* W) {
    REQUIRE_NON_NULL(x);
    REQUIRE_NON_NULL(W);
    IS_VECTOR(x);

    size_t nbElements = x->size1 * x->size2;
    gsl_matrix* out = gsl_matrix_calloc(W->size1, W->size1 * (nbElements + 1));
    if(!out) return NULL;

    for(size_t i = 0; i < W->size1; i++) {
        for(size_t j = 0; j < nbElements; j++) {
            double value;
            if(x->size1 > x->size2) {
                value = gsl_matrix_get(x, j, 0);
            } else {
                value = gsl_matrix_get(x, 0, j);
            }
            gsl_matrix_set(out, i, i * (nbElements + 1) + j, value);
        }
        gsl_matrix_set(out, i, i * (nbElements + 1) + nbElements, 1.0);
    }

    return out;
}

gsl_matrix** backpropagation(gsl_matrix* x, gsl_matrix* y, gsl_matrix** allW, size_t nbLayers) {
    REQUIRE_NON_NULL(x);
    REQUIRE_NON_NULL(y);
    REQUIRE_NON_NULL(allW);
    IS_VECTOR(x);
    IS_VECTOR(y);

    gsl_matrix** inputs = calloc(nbLayers, sizeof(gsl_matrix*));
    gsl_matrix** combinations = calloc(nbLayers, sizeof(gsl_matrix*));
    gsl_matrix* temp = gsl_matrix_calloc(x->size1, x->size2);
    if(!inputs || !combinations || !temp || gsl_matrix_memcpy(temp, x)) {
        free(inputs);
        free(combinations);
        if(!temp) gsl_matrix_free(temp);
        return NULL;
    }

    // Forward propagation 
    x = temp;
    for(size_t i = 0; i < nbLayers; i++) {
        inputs[i] = x;
        gsl_matrix* comb = affine(x, allW[i]);
        combinations[i] = comb;
        x = i == nbLayers - 1 ? identity(comb) : relu(comb);
    }
    gsl_matrix* currJac = dL2Dx(x, y);
    gsl_matrix_free(x);

    // Backpropagation
    gsl_matrix** out = calloc(nbLayers, sizeof(gsl_matrix*));
    if(!out) {
        destroyMatricesArray(inputs, nbLayers);
        destroyMatricesArray(combinations, nbLayers);
    }
    for(size_t r = 0; r < nbLayers; r++) {
        // Go in reverse order
        size_t i = nbLayers - 1 - r;

        gsl_matrix* comb = combinations[i];
        gsl_matrix* W = allW[i];

        gsl_matrix* multiplier = nbLayers - 1 ? dIdentityDx(comb) : dReluDx(comb);
        gsl_matrix* newJac = gsl_matrix_calloc(currJac->size1, multiplier->size2);

        if(gsl_blas_dgemm(CblasNoTrans, CblasNoTrans, 1.0, currJac, multiplier, 0.0, newJac)) {
            gsl_matrix_free(multiplier);
            gsl_matrix_free(newJac);
            gsl_matrix_free(currJac);
            destroyMatricesArray(inputs, nbLayers);
            destroyMatricesArray(combinations, nbLayers);
            return NULL;
        }

        gsl_matrix_free(multiplier);
        gsl_matrix_free(currJac);
        currJac = newJac;

        multiplier = dAffineDw(inputs[i], W);
        gsl_matrix* Jw = gsl_matrix_calloc(currJac->size1, multiplier->size2);
        if(gsl_blas_dgemm(CblasNoTrans, CblasNoTrans, 1.0, currJac, multiplier, 0.0, Jw)) {
            gsl_matrix_free(multiplier);
            gsl_matrix_free(Jw);
            gsl_matrix_free(currJac);
            destroyMatricesArray(inputs, nbLayers);
            destroyMatricesArray(combinations, nbLayers);
            return NULL;
        }
        gsl_matrix_free(multiplier);
        
        gsl_matrix* reshaped = reshape(Jw, W->size1, W->size2);
        gsl_matrix_free(Jw);
        if(!reshaped) {
            gsl_matrix_free(currJac);
            destroyMatricesArray(inputs, nbLayers);
            destroyMatricesArray(combinations, nbLayers);
            return NULL;
        }
        out[i] = reshaped;

        multiplier = dAffineDx(W);
        newJac = gsl_matrix_calloc(currJac->size1, multiplier->size2);
        if(gsl_blas_dgemm(CblasNoTrans, CblasNoTrans, 1.0, currJac, multiplier, 0.0, newJac)) {
            gsl_matrix_free(multiplier);
            gsl_matrix_free(newJac);
            gsl_matrix_free(currJac);
            destroyMatricesArray(inputs, nbLayers);
            destroyMatricesArray(combinations, nbLayers);
            return NULL;
        }
        gsl_matrix_free(multiplier);
        gsl_matrix_free(currJac);
        currJac = newJac;
    }
    
    gsl_matrix_free(currJac);
    destroyMatricesArray(inputs, nbLayers);
    destroyMatricesArray(combinations, nbLayers);

    return out;
}

gsl_matrix** train(gsl_matrix* trainInput, gsl_matrix* trainOutput, int* dimensions, size_t nbDimensions, int numEpoch, double learningRate) {
    REQUIRE_NON_NULL(trainInput);
    REQUIRE_NON_NULL(trainOutput);
    REQUIRE_NON_NULL(dimensions);
    
    gsl_matrix** allW = initNetwork(dimensions, nbDimensions);
    size_t nbSamples = trainInput->size1;
    size_t nbLayers = nbDimensions - 1;
    // Loop over all epochs
    for(size_t i = 0; i < numEpoch; i++) {
        // Loop over all samples
        for(size_t r = 0; r < nbSamples; r++) {
            gsl_matrix inRow = gsl_matrix_const_submatrix(trainInput, r, 0, 1, trainInput->size2).matrix;
            gsl_matrix outRow = gsl_matrix_const_submatrix(trainOutput, r, 0, 1, trainOutput->size2).matrix;
            
            gsl_matrix* tInRow = gsl_matrix_calloc(inRow.size2, inRow.size1);
            gsl_matrix* tOutRow = gsl_matrix_calloc(outRow.size2, outRow.size1);
            if(!tInRow || !tOutRow || gsl_matrix_transpose_memcpy(tInRow, &inRow) || gsl_matrix_transpose_memcpy(tOutRow, &outRow)) {
                if(tInRow) gsl_matrix_free(tInRow);
                if(tOutRow) gsl_matrix_free(tOutRow);
                destroyMatricesArray(allW, nbLayers);
                return NULL;
            }
            
            gsl_matrix** jacs = backpropagation(tInRow, tOutRow, allW, nbLayers);
            gsl_matrix_free(tInRow);
            gsl_matrix_free(tOutRow);

            // Update weights
            for(size_t j = 0; j < nbLayers; j++) {
                gsl_matrix* jac = jacs[j];

                if(gsl_matrix_scale(jac, learningRate) || gsl_matrix_sub(allW[j], jac)) {
                    destroyMatricesArray(allW, nbLayers);
                    destroyMatricesArray(jacs, nbLayers);
                    return NULL;
                }
            }
            destroyMatricesArray(jacs, nbLayers);
        }
    }

    return allW;
}

double sum(const gsl_matrix* m) {
    double total = 0.0;

    for(size_t i = 0; i < m->size1; i++) {
        for(size_t j = 0; j < m->size2; j++) {
            total += gsl_matrix_get(m, i, j);
        }
    }

    return total;
}

void elementPowerTwo(gsl_matrix* m) {
    for(size_t i = 0; i < m->size1; i++) {
        for(size_t j = 0; j < m->size2; j++) {
            double old = gsl_matrix_get(m, i, j);
            gsl_matrix_set(m, i, j, old * old);
        }
    }
}

gsl_matrix* normalize(const gsl_matrix* m) {
    REQUIRE_NON_NULL(m);

    gsl_matrix* out = gsl_matrix_calloc(m->size1, m->size2);
    if(!out) return NULL;

    if(gsl_matrix_memcpy(out, m)) {
        gsl_matrix_free(out);
        return NULL;
    }

    size_t nbElements = m->size1 * m->size2;
    double totalMean = sum(m) / nbElements;

    if(gsl_matrix_add_constant(out, -totalMean)) {
        gsl_matrix_free(out);
        return NULL;
    }

    elementPowerTwo(out);
    double totalStd = sqrt(sum(out) / nbElements);

    if(gsl_matrix_memcpy(out, m)) {
        gsl_matrix_free(out);
        return NULL;
    }

    if(gsl_matrix_add_constant(out, -totalMean)) {
        gsl_matrix_free(out);
        return NULL;
    }

    if(totalStd == 0) {
        return out;
    }

    if(gsl_matrix_scale(out, 1.0 / totalStd)) {
        gsl_matrix_free(out);
        return NULL;
    }

    return out;
}

gsl_matrix* nn(const gsl_matrix* x, const gsl_matrix** allW, size_t nbW) {
    REQUIRE_NON_NULL(x);
    REQUIRE_NON_NULL(allW);
    IS_VECTOR(x);
    
    gsl_matrix* out = gsl_matrix_calloc(x->size1, x->size2);
    if(!out) return NULL;

    if(gsl_matrix_memcpy(out, x)) {
        gsl_matrix_free(out);
        return NULL;
    }
    
    for(size_t i = 0; i < nbW; i++) {
        gsl_matrix* affined = affine(out, allW[i]);
        gsl_matrix* next = relu(affined);

        gsl_matrix_free(out);
        gsl_matrix_free(affined);

        out = next;
    }

    gsl_matrix* identitied = identity(out);
    gsl_matrix_free(out);
    out = identitied;

    return out;
}