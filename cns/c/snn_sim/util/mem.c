
#include "mem.h"

void replaceDoubleMem(double *src, doubleVector *v) {
//    free(src);
    src = (double*) malloc( v->size * sizeof(double) );
    memcpy(src, v->array, v->size);
}

void replaceIndMem(size_t *src, indVector *v) {
//    free(src);
    src = (size_t*) malloc( v->size * sizeof(size_t) );
    memcpy(src, v->array, v->size);
}
