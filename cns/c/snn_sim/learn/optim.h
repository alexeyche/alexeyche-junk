#ifndef LEARN_OPTIM_H
#define LEARN_OPTIM_H

#include <learn.h>


typedef struct {
    learn_t base;
    
    indLList **learn_syn_ids;
    double **C;
    double *B;

    doubleVector **stat_B;
    doubleVector ***stat_C;
    double *eligibility_trace;
} TOptimalSTDP;

TOptimalSTDP* init_TOptimalSTDP(struct SRMLayer *l);
void toStartValues_TOptimalSTDP(learn_t *ls_t);
void propagateSynSpike_TOptimalSTDP(learn_t *ls_t, const size_t *ni, const struct SynSpike *sp, const Constants *c);
void trainWeightsStep_TOptimalSTDP(learn_t *ls_t, const double *u, const double *p, const double *M, const size_t *ni, const struct Sim *s);
void resetValues_TOptimalSTDP(learn_t *ls_t, const size_t *ni);
void free_TOptimalSTDP(learn_t *ls_t);

#endif
