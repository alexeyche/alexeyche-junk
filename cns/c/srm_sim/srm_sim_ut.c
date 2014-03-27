
#include <matrix.h>
#include <util/util_vector.h>

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

void test_vector(void) {
    doubleVector *dv = TEMPLATE(createVector,double)();
    for(size_t eli=0; eli < 100; eli++) {
        TEMPLATE(insertVector,double)(dv, (double)eli);
    }
    printf("old size: %zu\n", dv->size);
    TEMPLATE(removeVector,double)(dv, 10);
    TEMPLATE(removeVector,double)(dv, 12);
    TEMPLATE(removeVector,double)(dv, dv->size-1);
    for(size_t eli=0; eli < dv->size; eli++) {
        if((eli == 2)||(eli == 3)) {
            TEMPLATE(removeVector,double)(dv, eli);
        }
        printf("%f, ", dv->array[eli]);
    }
    printf("new size: %zu\n", dv->size);
//    TEMPLATE(removeVector,double)(dv, );
    for(size_t eli=0; eli < dv->size; eli++) {
        printf("%f, ", dv->array[eli]);
    }
    printf("\n");
    TEMPLATE(deleteVector,double)(dv);
}

void main(void) {
    test_matrix();
//    test_sparse_matrix();
    test_vector();
}

