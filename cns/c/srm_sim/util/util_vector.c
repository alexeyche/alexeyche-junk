
#include "util_vector.h"

#include <templates_clean.h>
#define T double
#include "util_vector_tmpl.c"

#include <templates_clean.h>
#define T ind
#include "util_vector_tmpl.c"

void printDoubleVector(doubleVector *dv) {
    for(size_t el_i=0; el_i < dv->size; el_i++) {
        printf("%f, ", dv->array[el_i]);    
    }
    printf("\n");
}
