#ifndef OPTIM_LBFGS_H
#define OPTIM_LBFGS_H

#include <core/util/matrix.h>

#include <lbfgs.h>

#include "filt_funcs.h"

typedef struct {
    Matrix *y;
    doubleVector *target;
    int jobs;
    int L;
} LbfgsInput;


Matrix *formMatrixFromFloatVal(const lbfgsfloatval_t *x, size_t nrow, size_t ncol);
lbfgsfloatval_t evaluate(void *instance, const lbfgsfloatval_t *x, lbfgsfloatval_t *g, const int n, const lbfgsfloatval_t step);
int progress(void *instance, const lbfgsfloatval_t *x, const lbfgsfloatval_t *g, const lbfgsfloatval_t fx, \
             const lbfgsfloatval_t xnorm, const lbfgsfloatval_t gnorm, const lbfgsfloatval_t step, int n, int k, int ls);

Matrix *runLbfgsOptim(Matrix *y, doubleVector *target, int L, int jobs, double epsilon);

#endif
