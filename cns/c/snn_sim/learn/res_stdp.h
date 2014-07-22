#ifndef LEARN_STDP_H
#define LEARN_STDP_H

#include <math.h>

#include <learn/learn.h>


struct LayerPoisson;
struct SynSpike;

typedef struct {
    learn_t base;
    
    indLList **learn_syn_ids;
    double **x_tr;
    double *y_tr;
    double *res;
    double **eligibility_trace;
    double *reward;

    doubleVector ***stat_x_tr;
    doubleVector **stat_y_tr;
    doubleVector **stat_res;
} TResourceSTDP;

TResourceSTDP* init_TResourceSTDP(struct LayerPoisson *l);
void toStartValues_TResourceSTDP(learn_t *ls_t);
void propagateSynSpike_TResourceSTDP(learn_t *ls_t, const size_t *ni, const struct SynSpike *sp, const Constants *c);
void trainWeightsStep_TResourceSTDP(learn_t *ls_t, const double *u, const double *p, const double *M, const size_t *ni, const struct SimContext *s);
void resetValues_TResourceSTDP(learn_t *ls_t, const size_t *ni);
void free_TResourceSTDP(learn_t *ls_t);
void serialize_TResourceSTDP(learn_t *ls_t, FileStream *file, const Constants *c);
void deserialize_TResourceSTDP(learn_t *ls_t, FileStream *file, const Constants *c);

void consumeResource(double *res, const double *dw, const Constants *c);
float Q_rsqrt( float number );

#endif
