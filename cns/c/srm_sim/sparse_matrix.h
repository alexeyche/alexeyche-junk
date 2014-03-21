#ifndef SPARSE_MATRIX_H
#define SPARSE_MATRIX_H

#include "matrix.h"
#include "util_gen.h"

// http://software.intel.com/sites/products/documentation/hpc/mkl/mklman/GUID-9FCEB1C4-670D-4738-81D2-F378013412B0.htm

typedef struct {
    doubleVector *values;
    indVector *row_ind; 
    indVector *col_ptr
    size_t nrow;
    size_t ncol;
} SparseMatrix;


SparseMatrix* createSparseMatrix();
void deleteSparseMatrix(SparseMatrix *sm);

void fillSparseMatrix(SparseMatrix *sm, Matrix *m, size_t col_offset);
void setSparseMatrixElement(SparseMatrix *sm, size_t i, size_t j, double val);
double getSparseMatrixElement(SparseMatrix *sm, size_t i, size_t j);

#endif
