
#include "sparse_matrix.h"


SparseMatrix* createSparseMatrix(size_t nr, size_t nc) {
    SparseMatrix *sm = (SparseMatrix*)malloc(sizeof(SparseMatrix));
    sm->values = TEMPLATE(createVector,double)(0);
    sm->row_ind = TEMPLATE(createVector,ind)(0);
    sm->col_ptr = TEMPLATE(createVector,ind)(nc);
    sm->nrow = nr;
    sm->ncol = nc;
    return(sm);
}

void setSparseMatrixCol(SparseMatrix *sm, size_t j, doubleVector *vals) {
    assert(j < sm->ncol);
    assert(sm->col_ptr->array[j] == 0);
    for(size_t m=0; m < sm->nrow; m++) {
        if(val[m] != 0) {
            TEMPLATE(insertVector,ind)(sm->row_ind, m);
            TEMPLATE(insertVector,double)(sm->values, val[m]);
            sm->col_ptr->array[j] = sm->col_ptr->array[j] + 1;
        }
    }
}

double getSparseMatrixElement(SparseMatrix *sm, size_t i, size_t j) {
    return( sm->values->array[  sm->row_ind[ sm->col_ptr->array[j] ] + sm->col_ptr->array[j] ] );
}

void fillSparseMatrix(SparseMatrix *sm, Matrix *m, size_t col_offset) {
    for(size_t i = 0; i < m->nrow; i++ ) { 
        for(size_t j = 0; j < m->ncol; j++) {
            setSparseMatrixElement(sm, i, j+col_offset, getMatrixElement(m, i, j) );            
        }
    }
}


void deleteSparseMatrix(SparseMatrix *sm) {
    TEMPLATE(deleteVector,ind)(sm->row_ind);
    TEMPLATE(deleteVector,ind)(sm->col_ptr);
    TEMPLATE(deleteVector,double)(sm->values);
}
