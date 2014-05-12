#ifndef SIM_RUNTIME_H
#define SIM_RUNTIME_H

#include <util/util_vector.h>

typedef struct {
    doubleVector *reset_timeline;
    doubleVector *pattern_classes;
    size_t timeline_iter;
    
    double Tmax;
} SimRuntime;



#endif
