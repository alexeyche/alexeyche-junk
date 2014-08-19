

#include "triple_stdp.h"

#include <layers/poisson.h>
#include <sim/sim.h>
#include <neuron_funcs.h>

TripleSTDP* init_TripleSTDP(LayerPoisson *l) {
    TripleSTDP *ls = (TripleSTDP*) malloc( sizeof(TripleSTDP) );    
    ls->base.l = l;
    
    if(l->stat->statLevel>0) {
        ls->stat_o_one = (doubleVector**) malloc( l->N*sizeof(doubleVector*));
        ls->stat_o_two = (doubleVector**) malloc( l->N*sizeof(doubleVector*));
        ls->stat_a_minus = (doubleVector**) malloc( l->N*sizeof(doubleVector*));
        ls->stat_r = (doubleVector***) malloc( l->N*sizeof(doubleVector**));
    }

    ls->learn_syn_ids = (indLList**) malloc( l->N*sizeof(indLList*));
    ls->o_one = (double*)malloc( l->N*sizeof(double));
    ls->o_two = (double*)malloc( l->N*sizeof(double));
    ls->r = (double**)malloc( l->N*sizeof(double*));
    ls->pacc = (double*)malloc( l->N*sizeof(double));
    for(size_t ni=0; ni<l->N; ni++) {
        ls->learn_syn_ids[ni] = TEMPLATE(createLList,ind)();
        ls->r[ni] = (double*) malloc(l->nconn[ni]*sizeof(double));
        if(l->stat->statLevel>0) {
            ls->stat_o_one[ni] = TEMPLATE(createVector,double)();
            ls->stat_o_two[ni] = TEMPLATE(createVector,double)();
            ls->stat_a_minus[ni] = TEMPLATE(createVector,double)();
            ls->stat_r[ni] = (doubleVector**) malloc( l->nconn[ni]*sizeof(doubleVector*));
            for(size_t con_i=0; con_i < l->nconn[ni]; con_i++) {
                ls->stat_r[ni][con_i] = TEMPLATE(createVector,double)();
            }

        }
    } 

    ls->base.toStartValues = &toStartValues_TripleSTDP;
    ls->base.propagateSynSpike = &propagateSynSpike_TripleSTDP;
    ls->base.trainWeightsStep = &trainWeightsStep_TripleSTDP;
    ls->base.resetValues = &resetValues_TripleSTDP;
    ls->base.free = &free_TripleSTDP;
    ls->base.serialize = &serialize_TripleSTDP;
    ls->base.deserialize = &deserialize_TripleSTDP;
    ls->base.saveStat = &saveStat_TripleSTDP;

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
//    double p_norm = ls->pacc[*ni]/c->tr_stdp->__sec_tau_average;
//    printf("%f\n", p_norm);
//    double Aminus = p_norm * p_norm * p_norm * c->tr_stdp->__Aminus_cube_delim_p_target;
    double Aminus = c->tr_stdp->Aminus;

    indLNode *act_node = NULL;
    while( (act_node = TEMPLATE(getNextLList,ind)(ls->learn_syn_ids[ *ni ]) ) != NULL ) {
        const size_t *syn_id = &act_node->value;
        if(l->syn_fired[ *ni ][ *syn_id ] == 1) {
            ls->r[*ni][*syn_id] += 1;
        }
        double dw = getLC(l,c)->lrate * ( c->tr_stdp->__Aplus * ls->r[*ni][*syn_id] * ls->o_two[ *ni ] * l->fired[ *ni ] - 
                                                                               Aminus * ls->o_one[*ni] * l->syn_fired[ *ni ][ *syn_id ]);
        
        dw = bound_grad(&l->W[ *ni ][ *syn_id ], &dw, &getLC(l,c)->wmax, c);
        l->W[ *ni ][ *syn_id ] += dw;

        if( (ls->r[ *ni ][ *syn_id ] < LEARN_ACT_TOL ) && (ls->r[ *ni ][ *syn_id ] > -LEARN_ACT_TOL )) {
            TEMPLATE(dropNodeLList,ind)(ls->learn_syn_ids[ *ni ], act_node);
        } else {
            ls->r[ *ni ][ *syn_id ] -= ls->r[ *ni ][ *syn_id ] / c->tr_stdp->tau_plus;
        }
    }
    if(l->fired[ *ni ] == 1) {
        ls->o_two[*ni] += 1;
    }
    ls->o_two[*ni] -= ls->o_two[*ni]/c->tr_stdp->tau_y;
    

    ls->pacc[*ni] -= ls->pacc[*ni]/c->tr_stdp->tau_average;
    ls->o_one[*ni] -= ls->o_one[*ni]/c->tr_stdp->tau_minus;

    if(l->stat->statLevel > 0) {
        TEMPLATE(insertVector,double)(ls->stat_o_one[ *ni ], ls->o_one[ *ni ]);
        TEMPLATE(insertVector,double)(ls->stat_o_two[ *ni ], ls->o_two[ *ni ]);
        TEMPLATE(insertVector,double)(ls->stat_a_minus[ *ni ], Aminus);
        if(l->stat->statLevel > 1) {
            for(size_t con_i=0; con_i<l->nconn[ *ni ]; con_i++) {
                TEMPLATE(insertVector,double)(ls->stat_r[ *ni ][ con_i ], ls->r[ *ni ][ con_i ]);
            }
        }
    }

}

void resetValues_TripleSTDP(learn_t *ls_t, const size_t *ni) {
}

void free_TripleSTDP(learn_t *ls_t) {
    TripleSTDP *ls = (TripleSTDP*)ls_t;
    LayerPoisson *l = ls->base.l;
    for(size_t ni=0; ni<l->N; ni++) {
        if(l->nconn[ni]>0) {
            free(ls->r[ni]);
        }

        if(l->stat->statLevel>0) {
            TEMPLATE(deleteVector,double)(ls->stat_o_one[ni]);
            TEMPLATE(deleteVector,double)(ls->stat_o_two[ni]);
            TEMPLATE(deleteVector,double)(ls->stat_a_minus[ni]);
            for(size_t con_i=0; con_i < l->nconn[ni]; con_i++) {
                TEMPLATE(deleteVector,double)(ls->stat_r[ni][con_i]);
            }                
            free(ls->stat_r[ni]);
        }
    }
    if(l->stat->statLevel>0) {
        free(ls->stat_o_one);
        free(ls->stat_o_two);
        free(ls->stat_a_minus);
        free(ls->stat_r);
    }
    free(ls->learn_syn_ids);
    free(ls->o_one);
    free(ls->o_two);
    free(ls->r);

    free(ls);
}

void serialize_TripleSTDP(learn_t *ls_t, FileStream *file, const Constants *c) {
    TripleSTDP *ls = (TripleSTDP*)ls_t;
    LayerPoisson *l = ls->base.l; 

    pMatrixVector *data = TEMPLATE(createVector,pMatrix)();    
    Matrix *pacc_m = createMatrix(l->N,1);
    for(size_t ni=0; ni < l->N; ni++) {
        setMatrixElement(pacc_m, ni, 0, ls->pacc[ni]);
    }
    TEMPLATE(insertVector,pMatrix)(data, pacc_m);
    saveMatrixList(file, data);
    TEMPLATE(deleteVector,pMatrix)(data);
}

#define TRIPLE_STDP_RULE_SERIALIZATION_SIZE 1
void deserialize_TripleSTDP(learn_t *ls_t, FileStream *file, const Constants *c) {
    TripleSTDP *ls = (TripleSTDP*)ls_t;
    LayerPoisson *l = ls->base.l; 
    pMatrixVector *data = readMatrixList(file, TRIPLE_STDP_RULE_SERIALIZATION_SIZE);
    
    Matrix *pacc_m = data->array[0];
    assert( (pacc_m->nrow == l->N) && (pacc_m->ncol == 1) );
    
    for(size_t ni=0; ni<l->N; ni++) {
        ls->pacc[ni] = getMatrixElement(pacc_m, ni, 0);
    }

    TEMPLATE(deleteVector,pMatrix)(data);
}

void saveStat_TripleSTDP(learn_t *ls_t, FileStream *file) {
    TripleSTDP *ls = (TripleSTDP*)ls_t;
    LayerPoisson *l = ls->base.l; 
    
    pMatrixVector *mv = TEMPLATE(createVector,pMatrix)();

    Matrix *m_o_one = vectorArrayToMatrix(ls->stat_o_one, l->N);
    Matrix *m_o_two = vectorArrayToMatrix(ls->stat_o_two, l->N);
    Matrix *m_a_minus = vectorArrayToMatrix(ls->stat_a_minus, l->N);

    TEMPLATE(insertVector,pMatrix)(mv, m_o_one);
    TEMPLATE(insertVector,pMatrix)(mv, m_o_two);
    TEMPLATE(insertVector,pMatrix)(mv, m_a_minus);
    for(size_t ni=0; ni < l->N; ni++) {
        Matrix *mr = vectorArrayToMatrix(ls->stat_r[ni], l->nconn[ni]);
        TEMPLATE(insertVector,pMatrix)(mv, mr);
    }
    saveMatrixList(file, mv);
    TEMPLATE(deleteVector,pMatrix)(mv);
}
