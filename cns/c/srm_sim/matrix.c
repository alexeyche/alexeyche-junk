
#include "matrix.h"

#include <templates_clean.h>
#define T pMatrix
#define DESTRUCT deleteMatrix
#include <util/util_vector_tmpl.c>



Matrix *createMatrix(size_t nr, size_t nc) {
    Matrix *m = (Matrix*)malloc(sizeof(Matrix));
    m->nrow = nr;
    m->ncol = nc;
    m->vals = (double*)malloc(m->nrow*m->ncol*sizeof(double));
    return(m);
}

void deleteMatrix(Matrix *m) {
    free(m->vals);
    free(m);
}

void setMatrixElement(Matrix *m, size_t i, size_t j, double val) {
    m->vals[j*m->nrow + i] = val;
}
double getMatrixElement(Matrix *m, size_t i, size_t j) {
    return m->vals[j*m->nrow + i];
}

void printMatrix(Matrix *m) {
    for(size_t mi=0; mi < m->nrow; mi++) {
        for(size_t mj=0; mj < m->ncol; mj++) {
            printf("%f ", getMatrixElement(m, mi, mj));
        }
        printf("\n");
    }
}

Matrix* vectorArrayToMatrix(doubleVector **dv, size_t size) {
    size_t col_max = 0;
    for(size_t ri=0; ri< size; ri++) {
        if(dv[ri]->size>col_max) {
            col_max = dv[ri]->size;
        }
    }
    Matrix *m = createMatrix(size, col_max);
    for(size_t i=0; i<size; i++) {
        for(size_t j=0; j<col_max; j++) {
            if(j < dv[i]->size) {
                setMatrixElement(m, i, j, dv[i]->array[j]);
            } else {
                setMatrixElement(m, i, j, 0);
            }
        }
    }
    return(m);        
}


