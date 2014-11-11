

#include "simple_stdp.h"

#include <core/layers/poisson.h>
#include <core/sim/sim.h>
#include <core/layers/neuron_funcs.h>

SimpleSTDP* init_SimpleSTDP(LayerPoisson *l) {
    SimpleSTDP *ls = (SimpleSTDP*) malloc( sizeof(SimpleSTDP) );    
    ls->base.l = l;

    ls->base.toStartValues = &toStartValues_SimpleSTDP;
    ls->base.propagateSynSpike = &propagateSynSpike_SimpleSTDP;
    ls->base.trainWeightsStep = &trainWeightsStep_SimpleSTDP;
    ls->base.resetValues = &resetValues_SimpleSTDP;
    ls->base.free = &free_SimpleSTDP;
    ls->base.serialize = &serialize_SimpleSTDP;
    ls->base.deserialize = &deserialize_SimpleSTDP;
    ls->base.saveStat = &saveStat_SimpleSTDP;

    return(ls);
}

void toStartValues_SimpleSTDP(learn_t *ls_t) {
}

void propagateSynSpike_SimpleSTDP(learn_t *ls_t, const size_t *ni, const struct SynSpike *sp, const Constants *c) {
}



void trainWeightsStep_SimpleSTDP(learn_t *ls_t, const double *u, const double *p, const double *M, const size_t *ni, const SimContext *s) {
    SimpleSTDP *ls = (SimpleSTDP*)ls_t;
    LayerPoisson *l = ls->base.l;
    const Constants *c = s->c; 
    if(l->fired[*ni] == 1.0) {
        indLNode *act_node;
        while( (act_node = TEMPLATE(getNextLList,ind)(l->active_syn_ids[ *ni ]) ) != NULL ) {
            const size_t *syn_id = &act_node->value;

            double dw = getLC(l,c)->lrate * ( exp( - l->W[*ni][*syn_id] ) *  l->syn[*ni][*syn_id] - getLC(l,c)->ltd_factor );
            dw = bound_grad(&l->W[ *ni ][ *syn_id ], &dw, &getLC(l,c)->wmax, c);
            l->W[*ni][*syn_id] += dw;
        }
    }
}

void resetValues_SimpleSTDP(learn_t *ls_t, const size_t *ni) {
}

void free_SimpleSTDP(learn_t *ls_t) {
    SimpleSTDP *ls = (SimpleSTDP*)ls_t;
    free(ls);
}

void serialize_SimpleSTDP(learn_t *ls_t, FileStream *file, const Sim *s) {
}
void deserialize_SimpleSTDP(learn_t *ls_t, FileStream *file, const Sim *s) {
}

void saveStat_SimpleSTDP(learn_t *ls_t, pMatrixVector *mv) {

}

