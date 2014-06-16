
#include "util_vector.h"

#include <util/templates_clean.h>
#define T double
#include "util_vector_tmpl.c"

#include <util/templates_clean.h>
#define T ind
#include "util_vector_tmpl.c"

#include <util/templates_clean.h>
#define T int
#include "util_vector_tmpl.c"


#include <util/templates_clean.h>
#define T uchar
#include "util_vector_tmpl.c"


void printDoubleVector(doubleVector *dv) {
    for(size_t el_i=0; el_i < dv->size; el_i++) {
        printf("%3.3f, ", dv->array[el_i]);    
    }
    printf("\n");
}

void printIndVector(indVector *v) {
    for(size_t i=0; i<v->size; i++) {
        printf("%zu, ", v->array[i]);
    }
    printf("\n");
}

int int_compare (const void * a, const void * b)
{
    int int_a = * ( (int*) a );
    int int_b = * ( (int*) b );
    
    if ( int_a == int_b ) return 0;
    else if ( int_a < int_b ) return -1;
    else return 1;
}

int double_compare(const void * a, const void * b)
{
    double double_a = * ( (double*) a );
    double double_b = * ( (double*) b );
    
    if ( double_a == double_b ) return 0;
    else if ( double_a < double_b ) return -1;
    else return 1;
}

int ind_compare(const void * a, const void * b)
{
    ind ind_a = * ( (ind*) a );
    ind ind_b = * ( (ind*) b );
    
    if ( ind_a == ind_b ) return 0;
    else if ( ind_a < ind_b ) return -1;
    else return 1;
}

void sortDoubleVector(doubleVector *v) {
    qsort(v->array, v->size, sizeof(double), double_compare);
}

void sortIntVector(intVector *v) {
    qsort(v->array, v->size, sizeof(int), int_compare);
}
void sortIndVector(indVector *v) {
    qsort(v->array, v->size, sizeof(ind), ind_compare);
}


