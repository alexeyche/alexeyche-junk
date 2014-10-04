
#include "optim_lbfgs.h"

lbfgsfloatval_t evaluate(void *instance, const lbfgsfloatval_t *x, lbfgsfloatval_t *g, const int n, const lbfgsfloatval_t step) {
    LbfgsInput* inp = (LbfgsInput*)instance;    
    
    Matrix *w = formMatrixFromFloatVal(x, inp->y->nrow, inp->L);
    double fx = calcError(inp->y, w, inp->target, inp->jobs);

    Matrix *dedw = calcErrorGrad(inp->y, w, inp->target, inp->jobs);
    for (size_t i=0; i<n; i++) {
        g[i] = dedw->vals[i];
    }
    return fx; 
}

int progress(void *instance, const lbfgsfloatval_t *x, const lbfgsfloatval_t *g, const lbfgsfloatval_t fx, \
                    const lbfgsfloatval_t xnorm, const lbfgsfloatval_t gnorm, const lbfgsfloatval_t step, int n, int k, int ls) {
    printf("Iteration %d:\n", k);
    printf(" fx = %f,  xnorm = %f, gnorm = %f, step = %f\n", fx, xnorm, gnorm, step);
    printf("\n");
    return 0;
}

Matrix *formMatrixFromFloatVal(const lbfgsfloatval_t *x, size_t nrow, size_t ncol) {
    Matrix *m = createMatrix(nrow, ncol);
    for(size_t i=0; i<nrow; i++) {
        for(size_t j=0; j<ncol; j++) {
            setMatrixElement(m, i, j, x[j*nrow + i]);
        }
    }
    return(m);
}

Matrix *runLbfgsOptim(Matrix *y, doubleVector *target, int L, int jobs, double epsilon) {
    int M = y->nrow;
    Matrix *w = createZeroMatrix(M, L);
    
    lbfgsfloatval_t fx;
    lbfgsfloatval_t *x = lbfgs_malloc(w->nrow*w->ncol);
    lbfgs_parameter_t param;

    if (x == NULL) {
        printf("ERROR: Failed to allocate a memory block for variables.\n");
        exit(1);
    }

    /* Initialize the variables. */
    for (size_t i = 0; i < w->nrow; i++) {
        for (size_t j = 0; j < w->ncol;j++) {
            x[j*w->nrow + i] = getMatrixElement(w, i, j);
        }    
    }
    lbfgs_parameter_init(&param);
    param.epsilon = epsilon;
    param.m = 20;

    LbfgsInput inp;
    inp.target = target;
    inp.y = y;
    inp.jobs = jobs;
    inp.L = w->ncol;
    int ret = lbfgs(w->nrow*w->ncol, x, &fx, evaluate, progress, (void*)&inp, &param);

    printf("L-BFGS optimization terminated with status code = %d\n", ret);
    printf("  fx = %f\n", fx);
    Matrix *w_opt = formMatrixFromFloatVal(x, w->nrow, w->ncol);
    
    lbfgs_free(x);

    return(w_opt);
}
