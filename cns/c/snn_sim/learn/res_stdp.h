#ifndef LEARN_STDP_H
#define LEARN_STDP_H

#include <learn.h>
#include <math.h>

#ifndef max
    #define max( a, b ) ( ((a) > (b)) ? (a) : (b) )
#endif

struct SRMLayer;
struct SynSpike;

typedef struct {
    learn_t base;
    
    indLList **learn_syn_ids;
    double **x_tr;
    double *y_tr;
    double *res;

    doubleVector ***stat_x_tr;
    doubleVector **stat_y_tr;
    doubleVector **stat_res;
} TResourceSTDP;

TResourceSTDP* init_TResourceSTDP(struct SRMLayer *l);
void toStartValues_TResourceSTDP(learn_t *ls_t);
void propagateSynSpike_TResourceSTDP(learn_t *ls_t, const size_t *ni, const struct SynSpike *sp, const Constants *c);
void trainWeightsStep_TResourceSTDP(learn_t *ls_t, const double *u, const double *p, const double *M, const size_t *ni, const Constants *c);
void resetValues_TResourceSTDP(learn_t *ls_t, const size_t *ni);
void free_TResourceSTDP(learn_t *ls_t);

#endif
