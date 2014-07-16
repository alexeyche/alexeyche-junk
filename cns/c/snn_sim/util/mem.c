
#include "mem.h"

double* copyDoubleMem(doubleVector *v) {
    double *new_src = (double*) malloc( v->size * sizeof(double) );
    for(size_t i=0; i<v->size; i++) {
        new_src[i] = v->array[i];
    }
    return(new_src);
}

size_t* copyIndMem(indVector *v) {
    size_t* new_src = (size_t*) malloc( v->size * sizeof(size_t) );
    for(size_t i=0; i<v->size; i++) {
        new_src[i] = v->array[i];
    }
    return(new_src);
}
