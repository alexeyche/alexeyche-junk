

#include "optim.h"

#include <layers/layer_poisson.h>
#include <sim/sim.h>

OptimalSTDP* init_OptimalSTDP(LayerPoisson *l) {
    OptimalSTDP *ls = (OptimalSTDP*) malloc( sizeof(OptimalSTDP));
    ls->base.l = l; 
    ls->learn_syn_ids = (indLList**) malloc( l->N*sizeof(indLList*));
    ls->B = (double*)malloc( l->N*sizeof(double));
    ls->C = (double**)malloc( l->N*sizeof(double*));
    ls->pacc = (double*)malloc( l->N*sizeof(double));
    
    if(l->stat->statLevel>1) {
        ls->stat_B = (doubleVector**) malloc( l->N*sizeof(doubleVector*));
        ls->stat_C = (doubleVector***) malloc( l->N*sizeof(doubleVector**));
    }
    ls->eligibility_trace = (double*) malloc( l->N*sizeof(double));
    for(size_t ni=0; ni<l->N; ni++) {
        ls->learn_syn_ids[ni] = TEMPLATE(createLList,ind)();
        ls->C[ni] = (double*) malloc(l->nconn[ni]*sizeof(double));
        ls->pacc[ni] = 0;
        ls->eligibility_trace[ni] = 0;
        if(l->stat->statLevel>1) {
            ls->stat_B[ni] = TEMPLATE(createVector,double)();
            ls->stat_C[ni] = (doubleVector**) malloc( l->nconn[ni]*sizeof(doubleVector*));
            for(size_t con_i=0; con_i < l->nconn[ni]; con_i++) {
                ls->stat_C[ni][con_i] = TEMPLATE(createVector,double)();
            }
        }
    }
    
    ls->base.toStartValues = &toStartValues_OptimalSTDP;
    ls->base.propagateSynSpike = &propagateSynSpike_OptimalSTDP;
    ls->base.trainWeightsStep = &trainWeightsStep_OptimalSTDP;
    ls->base.resetValues = &resetValues_OptimalSTDP;
    ls->base.free = &free_OptimalSTDP;
    ls->base.serialize = &serialize_OptimalSTDP;
    ls->base.deserialize = &deserialize_OptimalSTDP;
    return(ls);
}

void toStartValues_OptimalSTDP(learn_t *ls_t) {
    OptimalSTDP *ls = (OptimalSTDP*)ls_t; 
    LayerPoisson *l = ls->base.l;
    
    for(size_t ni=0; ni<l->N; ni++) {
        ls->B[ni] = 0;
        for(size_t syn_i=0; syn_i<l->nconn[ni]; syn_i++) {
            ls->C[ni][syn_i] = 0;
        }
    }
}

void propagateSynSpike_OptimalSTDP(learn_t *ls_t, const size_t *ni, const SynSpike *sp, const Constants *c) {
    OptimalSTDP *ls = (OptimalSTDP*)ls_t;
    if( (ls->C[ *ni ][ sp->syn_id ] < LEARN_ACT_TOL ) && (ls->C[ *ni ][ sp->syn_id ] > -LEARN_ACT_TOL ) ) {
        TEMPLATE(addValueLList,ind)(ls->learn_syn_ids[*ni], sp->syn_id);
    }
}

void trainWeightsStep_OptimalSTDP(learn_t *ls_t, const double *u, const double *p, const double *M, const size_t *ni, const SimContext *s) {
    OptimalSTDP *ls = (OptimalSTDP*)ls_t;
    LayerPoisson *l = ls->base.l; 
    const Constants *c = s->c;

    if(l->fired[ *ni ] == 1) {
        ls->pacc[*ni] += 1;
    }
    if(getLC(l,c)->learn) {
        ls->B[ *ni ] = B_calc( &l->fired[ *ni ], p, &ls->pacc[ *ni ], c);
    
        indLNode *act_node = NULL;
        while( (act_node = TEMPLATE(getNextLList,ind)(ls->learn_syn_ids[ *ni ]) ) != NULL ) {
    //            for(size_t con_i=0; con_i < l->nconn[ *ni ]; con_i++) {
    //                const size_t *syn_id = &con_i;
            const size_t *syn_id = &act_node->value;

    //                if( (l->C[ *ni ][ *syn_id ] == 0) && (l->syn[ *ni ][ *syn_id ] == 0) ) continue;
            double p_stroke = l->prob_fun_stroke(u,c);
            double dC = C_calc( &l->fired[ *ni ], p, &p_stroke, u, M, &l->syn[ *ni ][ *syn_id ], c); // * l->syn_spec[ *ni ][ *syn_id ];
            ls->C[ *ni ][ *syn_id ] += -ls->C[ *ni ][ *syn_id ]/c->tc + dC;
    //                printf("dC: %f C: %f, params: %d %f %f %f %f\n", dC, l->C[ *ni ][ *syn_id ], l->fired[ *ni ], p, u, l->syn[ *ni ][ *syn_id ], M);
            
    #if RATE_NORM == PRESYNAPTIC
            double dw = getLC(l,c)->lrate*( ls->C[ *ni ][ *syn_id ]*ls->B[ *ni ] -  \
                                        getLC(l,c)->weight_decay_factor * l->syn_fired[ *ni ][ *syn_id ] * l->W[ *ni ][ *syn_id ] );
    #elif RATE_NORM == POSTSYNAPTIC                
            double dw = getLC(l,c)->lrate*( l->C[ *ni ][ *syn_id ]*l->B[ *ni ] -  \
                                        getLC(l,c)->weight_decay_factor * (l->fired[ *ni ] + l->syn_fired[ *ni ][ *syn_id ]) * l->W[ *ni ][ *syn_id ] );
    #endif               
            double wmax = getLC(l,c)->wmax;
            dw = bound_grad(&l->W[ *ni ][ *syn_id ], &dw, &wmax, c);
//            if(l->syn_spec[*ni][*syn_id]>0) {
                l->W[ *ni ][ *syn_id ] += dw;
//            } else {
//                l->W[ *ni ][ *syn_id ] += dw*0.1;
//            }
    
            
            if( (ls->C[ *ni ][ *syn_id ] < LEARN_ACT_TOL ) && (ls->C[ *ni ][ *syn_id ] > -LEARN_ACT_TOL ) && 
                                                              (dC < LEARN_ACT_TOL ) && (dC > -LEARN_ACT_TOL ) ) {
    
                TEMPLATE(dropNodeLList,ind)(ls->learn_syn_ids[ *ni ], act_node);
            }
            if( isnan(dw) ) { 
                printf("\nFound bad value\n");
                printf("nid: %zu, p: %f, u: %f, B: %f, pacc: %f, C: %f, W: %f, dw: %f\n", *ni, *p, *u, ls->B[ *ni ], ls->pacc[ *ni ], ls->C[ *ni ][ *syn_id ], l->W[ *ni ][ *syn_id ], dw);
                printf("C params: Yspike: %d, synapse: %f, dC: %f, p': %f\n", l->fired[ *ni],l->syn[ *ni ][ *syn_id ], dC, pstroke(u,c));
                exit(1);
            }
    
       }
   }
   ls->pacc[ *ni ] -= ls->pacc[ *ni ]/c->mean_p_dur; 

   if(l->stat->statLevel>1) {
        TEMPLATE(insertVector,double)(ls->stat_B[ *ni ], ls->B[ *ni ]);
        for(size_t con_i=0; con_i<l->nconn[ *ni ]; con_i++) {
            TEMPLATE(insertVector,double)(ls->stat_C[ *ni ][ con_i ], ls->C[ *ni ][ con_i ]);
        }
    }        
}
void resetValues_OptimalSTDP(learn_t *ls_t, const size_t *ni) {
    OptimalSTDP *ls = (OptimalSTDP*)ls_t;
    LayerPoisson *l = ls->base.l; 
    
    ls->B[ *ni ] = 0;
    for(size_t con_i=0; con_i < l->nconn[ *ni ]; con_i++) {
       ls->C[ *ni ][con_i] = 0;
    }
    indLNode *act_node = NULL;
    while( (act_node = TEMPLATE(getNextLList,ind)(ls->learn_syn_ids[ *ni ]) ) != NULL ) {
       TEMPLATE(dropNodeLList,ind)(ls->learn_syn_ids[*ni], act_node);
    }
}

void free_OptimalSTDP(learn_t *ls_t) {
    OptimalSTDP *ls = (OptimalSTDP*)ls_t;
    LayerPoisson *l = ls->base.l; 
    for(size_t ni=0; ni<l->N; ni++) {
        if(l->nconn[ni]>0) {
            free(ls->C[ni]);
        }
        TEMPLATE(deleteLList,ind)(ls->learn_syn_ids[ni]);
        if(l->stat->statLevel>1) {
            TEMPLATE(deleteVector,double)(ls->stat_B[ni]);
            for(size_t con_i=0; con_i < l->nconn[ni]; con_i++) {
                TEMPLATE(deleteVector,double)(ls->stat_C[ni][con_i]);
            }                
            free(ls->stat_C[ni]);
        }
    }
    if(l->stat->statLevel>1) {
        free(ls->stat_B);
        free(ls->stat_C);
    }

    free(ls->learn_syn_ids);
    free(ls->B);
    free(ls->C);
    free(ls);
}

void serialize_OptimalSTDP(learn_t *ls_t, FileStream *file, const Constants *c) {
    OptimalSTDP *ls = (OptimalSTDP*)ls_t;
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

#define OPTIMAL_STDP_RULE_SERIALIZATION_SIZE 1
void deserialize_OptimalSTDP(learn_t *ls_t, FileStream *file, const Constants *c) {
    OptimalSTDP *ls = (OptimalSTDP*)ls_t;
    LayerPoisson *l = ls->base.l; 
    pMatrixVector *data = readMatrixList(file, OPTIMAL_STDP_RULE_SERIALIZATION_SIZE);
    
    Matrix *pacc_m = data->array[0];
    assert( (pacc_m->nrow == l->N) && (pacc_m->ncol == 1) );
    
    for(size_t ni=0; ni<l->N; ni++) {
        ls->pacc[ni] = getMatrixElement(pacc_m, ni, 0);
    }

    TEMPLATE(deleteVector,pMatrix)(data);
}

