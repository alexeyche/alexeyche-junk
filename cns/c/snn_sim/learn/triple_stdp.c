

#include "triple_stdp.h"

#include <layers/poisson.h>
#include <sim/sim.h>
#include <neuron_funcs.h>

TripleSTDP* init_TripleSTDP(LayerPoisson *l) {
    TripleSTDP *ls = (TripleSTDP*) malloc( sizeof(TripleSTDP) );    
    ls->base.l = l;

    ls->learn_syn_ids = (indLList**) malloc( l->N*sizeof(indLList*));
    ls->o_one = (double*)malloc( l->N*sizeof(double));
    ls->o_two = (double*)malloc( l->N*sizeof(double));
    ls->r = (double**)malloc( l->N*sizeof(double*));
    ls->pacc = (double*)malloc( l->N*sizeof(double));
    for(size_t ni=0; ni<l->N; ni++) {
        ls->learn_syn_ids[ni] = TEMPLATE(createLList,ind)();
        ls->r[ni] = (double*) malloc(l->nconn[ni]*sizeof(double));
    } 

    ls->base.toStartValues = &toStartValues_TripleSTDP;
    ls->base.propagateSynSpike = &propagateSynSpike_TripleSTDP;
    ls->base.trainWeightsStep = &trainWeightsStep_TripleSTDP;
    ls->base.resetValues = &resetValues_TripleSTDP;
    ls->base.free = &free_TripleSTDP;
    ls->base.serialize = &serialize_TripleSTDP;
    ls->base.deserialize = &deserialize_TripleSTDP;

    return(ls);
}

void toStartValues_TripleSTDP(learn_t *ls_t) {
    TripleSTDP *ls = (TripleSTDP*)ls_t;
    LayerPoisson *l = ls->base.l;

    for(size_t ni=0; ni < l->N; ni++) {
        ls->o_one[ni] = 0.0;
        ls->o_two[ni] = 0.0;
        ls->pacc[ni] = 0.0;
        for(size_t con_i=0; con_i < l->nconn[ni]; con_i++) {
            ls->r[ni][con_i] = 0.0;
        }
    }
}

void propagateSynSpike_TripleSTDP(learn_t *ls_t, const size_t *ni, const struct SynSpike *sp, const Constants *c) {
    TripleSTDP *ls = (TripleSTDP*)ls_t;
    if( (ls->r[ *ni ][ sp->syn_id ] < LEARN_ACT_TOL ) && (ls->r[ *ni ][ sp->syn_id ] > -LEARN_ACT_TOL ) ) {
        TEMPLATE(addValueLList,ind)(ls->learn_syn_ids[*ni], sp->syn_id);
    }
}



void trainWeightsStep_TripleSTDP(learn_t *ls_t, const double *u, const double *p, const double *M, const size_t *ni, const SimContext *s) {
    TripleSTDP *ls = (TripleSTDP*)ls_t;
    LayerPoisson *l = ls->base.l;
    const Constants *c = s->c;

    if(l->fired[ *ni ] == 1) {
        ls->pacc[*ni] += 1;
        ls->o_one[*ni] += 1;
    }
    if(getLC(l,c)->learn) {
        double Aminus = ls->pacc[*ni] * ls->pacc[*ni] * ls->pacc[*ni] * c->tr_stdp->__Aminus_cube_delim_p_target;

        indLNode *act_node = NULL;
        while( (act_node = TEMPLATE(getNextLList,ind)(ls->learn_syn_ids[ *ni ]) ) != NULL ) {
            const size_t *syn_id = &act_node->value;
            if(l->syn_fired[ *ni ][ *syn_id ] == 1) {
                ls->r[*ni][*syn_id] += 1;
            }
            double dw = getLC(l,c)->lrate * ( c->tr_stdp->__Aplus * ls->r[*ni][*syn_id] * ls->o_two[ *ni ] * l->fired[ *ni ] - 
                                                                                   Aminus * ls->o_one[*ni] * l->syn_fired[ *ni ][ *syn_id ]);
            if( (ls->r[ *ni ][ *syn_id ] < LEARN_ACT_TOL ) && (ls->r[ *ni ][ *syn_id ] > -LEARN_ACT_TOL )) {
    
                TEMPLATE(dropNodeLList,ind)(ls->learn_syn_ids[ *ni ], act_node);
            }
            ls->r[ *ni ][ *syn_id ] -= ls->r[ *ni ][ *syn_id ] / c->tr_stdp->tau_plus;
        }
        if(l->fired[ *ni ] == 1) {
            ls->o_two[*ni] += 1;
        }
        ls->o_two[*ni] -= ls->o_two[*ni]/c->tr_stdp->tau_y;
    }
    ls->pacc[*ni] -= ls->pacc[*ni]/c->tr_stdp->tau_average;
    ls->o_one[*ni] -= ls->o_one[*ni]/c->tr_stdp->tau_minus;
}

void resetValues_TripleSTDP(learn_t *ls_t, const size_t *ni) {
}

void free_TripleSTDP(learn_t *ls_t) {
    TripleSTDP *ls = (TripleSTDP*)ls_t;
    free(ls);
}

void serialize_TripleSTDP(learn_t *ls_t, FileStream *file, const Constants *c) {
}
void deserialize_TripleSTDP(learn_t *ls_t, FileStream *file, const Constants *c) {
}

