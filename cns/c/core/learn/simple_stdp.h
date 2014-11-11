#ifndef SIMPLE_STDP_H
#define SIMPLE_STDP_H

#include <core/learn/learn.h>


struct LayerPoisson;
struct SynSpike;
struct Sim;

typedef struct {
    learn_t base;
    
} SimpleSTDP;

SimpleSTDP* init_SimpleSTDP(struct LayerPoisson *l);
void toStartValues_SimpleSTDP(learn_t *ls_t);
void propagateSynSpike_SimpleSTDP(learn_t *ls_t, const size_t *ni, const struct SynSpike *sp, const Constants *c);
void trainWeightsStep_SimpleSTDP(learn_t *ls_t, const double *u, const double *p, const double *M, const size_t *ni, const struct SimContext *s);
void resetValues_SimpleSTDP(learn_t *ls_t, const size_t *ni);
void free_SimpleSTDP(learn_t *ls_t);
void serialize_SimpleSTDP(learn_t *ls_t, FileStream *file, const struct Sim *s);
void deserialize_SimpleSTDP(learn_t *ls_t, FileStream *file, const struct Sim *s);
void saveStat_SimpleSTDP(learn_t *ls_t, pMatrixVector *mv);

#endif
