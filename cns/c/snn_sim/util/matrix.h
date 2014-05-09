#ifndef MATRIX_H
#define MATRIX_H

#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <templates.h>
#include <util/util_vector.h>

typedef struct {
    double *vals;
    size_t nrow;
    size_t ncol;
} Matrix;

typedef Matrix* pMatrix;

#include <templates_clean.h>
#define T pMatrix
#define DESTRUCT deleteMatrix
#include <util/util_vector_tmpl.h>

Matrix *createMatrix(size_t nr, size_t nc);
Matrix *copyMatrix(Matrix *m);
void setMatrixElement(Matrix *m, size_t i, size_t j, double val);
double getMatrixElement(Matrix *m, size_t i, size_t j);
void deleteMatrix(Matrix *m);
void printMatrix(Matrix *m);
Matrix *vectorArrayToMatrix(doubleVector **dv, size_t size);


#endif
