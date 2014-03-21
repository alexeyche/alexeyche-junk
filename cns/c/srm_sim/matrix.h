#ifndef MATRIX_H
#define MATRIX_H

#include <stddef.h>
#include <stdlib.h>

typedef struct {
    double *vals;
    size_t nrow;
    size_t ncol;
} Matrix;

Matrix *createMatrix(size_t nr, size_t nc);
void setMatrixElement(Matrix *m, size_t i, size_t j, double val);
double getMatrixElement(Matrix *m, size_t i, size_t j);
void deleteMatrix(Matrix *m);

#endif
