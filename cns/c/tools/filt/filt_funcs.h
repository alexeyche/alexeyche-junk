#ifndef FILT_FUNCS_H
#define FILT_FUNCS_H

#include <pthread.h>

#include <core/util/matrix.h>


typedef struct {
    int first;
    int last;
    double acc;
    Matrix *w;
    Matrix *y;
    doubleVector *target;

    Matrix *dedw;
} FiltWorker;


double conv(size_t i, Matrix *y, Matrix *w);

void *error_routine(void *args);
void *error_grad_routine(void *args);

double calcError(Matrix *y, Matrix *w, doubleVector *target, int jobs);
Matrix* calcErrorGrad(Matrix *y, Matrix *w, doubleVector *target, int jobs);


#endif
