
#include "mem.h"

double* copyDoubleMem(doubleVector *v) {
    double *new_src = (double*) malloc( v->size * sizeof(double) );
    memcpy(new_src, v->array, v->size);
    return(new_src);
}

size_t* copyIndMem(indVector *v) {
    size_t* new_src = (size_t*) malloc( v->size * sizeof(size_t) );
    memcpy(new_src, v->array, v->size);
    return(new_src);
}
