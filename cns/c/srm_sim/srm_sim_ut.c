
#include "core.h"

void test_matrix() {
    Matrix *m = createMatrix(50, 25);
    for(size_t i = 0; i<50; i++) {
        for(size_t j = 0; j<25; j++) {
            setMatrixElement(m, i, j, i*j);
        } 
    }
    for(size_t i = 0; i<50; i++) {
        printf("%zu: ", i);
        for(size_t j = 0; j<25; j++) {
            double v = getMatrixElement(m, i, j);
            printf("%zu = %f ", j,v);
        } 
        printf("\n");
    }
    deleteMatrix(m);
}

//void test_sparse_matrix() {
//    SparseMatrix *sm = createSparseMatrix();
//    Matrix *m = createMatrix(50, 25);
//    for(size_t i = 0; i<50; i++) {
//        for(size_t j = 0; j<25; j++) {
//            setMatrixElement(m, i, j, i*j);
//        } 
//    }
//    fillSparseMatrix(sm, m, 0);
//    fillSparseMatrix(sm, m, 25);
//    
//    for(size_t i = 0; i<50; i++) {
//        printf("%zu: ", i);
//        for(size_t j = 0; j<50; j++) {
//            double v = getSparseMatrixElement(sm, i, j);
//            printf("%zu = %f ", j,v);
//        } 
//    }
//    
//    deleteMatrix(m);
//    deleteSparseMatrix(sm);
//}

void main(void) {
    test_matrix();
//    test_sparse_matrix();
}

