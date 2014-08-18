#ifndef TRIPLE_STDP_H
#define TRIPLE_STDP_H

#include <math.h>

#include <learn/learn.h>


struct LayerPoisson;
struct SynSpike;

typedef struct {
    learn_t base;
    
    indLList **learn_syn_ids;
    double **r;
    double *o_one;
    double *o_two;
    double *pacc;    
} TripleSTDP;

TripleSTDP* init_TripleSTDP(struct LayerPoisson *l);
void toStartValues_TripleSTDP(learn_t *ls_t);
void propagateSynSpike_TripleSTDP(learn_t *ls_t, const size_t *ni, const struct SynSpike *sp, const Constants *c);
void trainWeightsStep_TripleSTDP(learn_t *ls_t, const double *u, const double *p, const double *M, const size_t *ni, const struct SimContext *s);
void resetValues_TripleSTDP(learn_t *ls_t, const size_t *ni);
void free_TripleSTDP(learn_t *ls_t);
void serialize_TripleSTDP(learn_t *ls_t, FileStream *file, const Constants *c);
void deserialize_TripleSTDP(learn_t *ls_t, FileStream *file, const Constants *c);


#endif
