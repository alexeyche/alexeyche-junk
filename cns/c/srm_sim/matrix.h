#ifndef MATRIX_H
#define MATRIX_H

#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>

#include <templates.h>

typedef struct {
    double *vals;
    size_t nrow;
    size_t ncol;
} Matrix;

typedef Matrix* pMatrix;

#ifdef T
#undef T
#endif
#define T pMatrix
#include <util/util_vector_tmpl.h>

Matrix *createMatrix(size_t nr, size_t nc);
void setMatrixElement(Matrix *m, size_t i, size_t j, double val);
double getMatrixElement(Matrix *m, size_t i, size_t j);
void deleteMatrix(Matrix *m);
void printMatrix(Matrix *m);

typedef struct {
    pMatrixVector *mlist;
} MatrixList;

MatrixList* createMatrixList();
void insertMatrixList(MatrixList *ml, Matrix *m);
void deleteMatrixList(MatrixList *ml);

#endif
