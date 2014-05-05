

#include "learn.h"

#include "layer.h"

TOptimalSTDP* init_TOptimalSTDP(SRMLayer *l) {
    TOptimalSTDP *ls = (TOptimalSTDP*) malloc( sizeof(TOptimalSTDP));
    ls->base.l = l; 
    ls->learn_syn_ids = (indLList**) malloc( l->N*sizeof(indLList*));
    ls->B = (double*)malloc( l->N*sizeof(double));
    ls->C = (double**)malloc( l->N*sizeof(double*));
    
    if(l->saveStat) {
        ls->stat_B = (doubleVector**) malloc( l->N*sizeof(doubleVector*));
        ls->stat_C = (doubleVector***) malloc( l->N*sizeof(doubleVector**));
    }
    for(size_t ni=0; ni<l->N; ni++) {
        ls->learn_syn_ids[ni] = TEMPLATE(createLList,ind)();
        ls->C[ni] = (double*) malloc(l->nconn[ni]*sizeof(double));
        if(l->saveStat) {
            ls->stat_B[ni] = TEMPLATE(createVector,double)();
            ls->stat_C[ni] = (doubleVector**) malloc( l->nconn[ni]*sizeof(doubleVector*));
            for(size_t con_i=0; con_i < l->nconn[ni]; con_i++) {
                TEMPLATE(createVector,double)(ls->stat_C[ni][con_i]);
            }
        }
    }
    
    ls->base.toStartValues = &toStartValues_TOptimalSTDP;
    ls->base.propagateSynSpike = &propagateSynSpike_TOptimalSTDP;
    ls->base.trainWeightsStep = &trainWeightsStep_TOptimalSTDP;
    ls->base.resetValues = &resetValues_TOptimalSTDP;
    ls->base.free = &free_TOptimalSTDP;
    return(ls);
}

void toStartValues_TOptimalSTDP(learn_t *ls_t) {
    TOptimalSTDP *ls = (TOptimalSTDP*)ls_t; 
    SRMLayer *l = ls->base.l;
    
    for(size_t ni=0; ni<l->N; ni++) {
        ls->B[ni] = 0;
        for(size_t syn_i=0; syn_i<l->nconn[ni]; syn_i++) {
            ls->C[ni][syn_i] = 0;
        }
    }
}

void propagateSynSpike_TOptimalSTDP(learn_t *ls_t, const size_t *ni, const SynSpike *sp, const Constants *c) {
    TOptimalSTDP *ls = (TOptimalSTDP*)ls_t;
    if( (ls->C[ *ni ][ sp->syn_id ] < LEARN_ACT_TOL ) && (ls->C[ *ni ][ sp->syn_id ] > -LEARN_ACT_TOL ) ) {
        TEMPLATE(addValueLList,ind)(ls->learn_syn_ids[*ni], sp->syn_id);
    }
}

void trainWeightsStep_TOptimalSTDP(learn_t *ls_t, const double *u, const double *p, const double *M, const size_t *ni, const Constants *c) {
    TOptimalSTDP *ls = (TOptimalSTDP*)ls_t;
    SRMLayer *l = ls->base.l; 

    ls->B[ *ni ] = B_calc( &l->fired[ *ni ], p, &l->pacc[ *ni ], c);
    indLNode *act_node = NULL;
    while( (act_node = TEMPLATE(getNextLList,ind)(ls->learn_syn_ids[ *ni ]) ) != NULL ) {
//            for(size_t con_i=0; con_i < l->nconn[ *ni ]; con_i++) {
//                const size_t *syn_id = &con_i;
        const size_t *syn_id = &act_node->value;
//                if( (l->C[ *ni ][ *syn_id ] == 0) && (l->syn[ *ni ][ *syn_id ] == 0) ) continue;
        double dC = C_calc( &l->fired[ *ni ], p, u, M, &l->syn[ *ni ][ *syn_id ], c);
        ls->C[ *ni ][ *syn_id ] += -ls->C[ *ni ][ *syn_id ]/c->tc + dC;
//                printf("dC: %f C: %f, params: %d %f %f %f %f\n", dC, l->C[ *ni ][ *syn_id ], l->fired[ *ni ], p, u, l->syn[ *ni ][ *syn_id ], M);
        
#if RATE_NORM == PRESYNAPTIC
        double dw = layerConstD(l, c->added_lrate)*( ls->C[ *ni ][ *syn_id ]*ls->B[ *ni ] -  \
                                    layerConstD(l, c->weight_decay_factor) * l->syn_fired[ *ni ][ *syn_id ] * l->W[ *ni ][ *syn_id ] );
#elif RATE_NORM == POSTSYNAPTIC                
        double dw = layerConstD(l, c->added_lrate)*( l->C[ *ni ][ *syn_id ]*l->B[ *ni ] -  \
                                    layerConstD(l, c->weight_decay_factor) * (l->fired[ *ni ] + l->syn_fired[ *ni ][ *syn_id ]) * l->W[ *ni ][ *syn_id ] );
#endif               
        double wmax = layerConstD(l, c->wmax);
        dw = bound_grad(&l->W[ *ni ][ *syn_id ], &dw, &wmax, c);
        l->W[ *ni ][ *syn_id ] += dw;
        
        if( (ls->C[ *ni ][ *syn_id ] < LEARN_ACT_TOL ) && (ls->C[ *ni ][ *syn_id ] > -LEARN_ACT_TOL ) && 
            (dC < LEARN_ACT_TOL ) && (dC > -LEARN_ACT_TOL ) ) {

            TEMPLATE(dropNodeLList,ind)(ls->learn_syn_ids[ *ni ], act_node);
        }
        if( isnan(dw) ) { 
            printf("\nFound bad value\n");
            printf("nid: %zu, p: %f, u: %f, B: %f, pacc: %f, C: %f, W: %f, dw: %f\n", *ni, *p, *u, ls->B[ *ni ], l->pacc[ *ni ], ls->C[ *ni ][ *syn_id ], l->W[ *ni ][ *syn_id ], dw);
            printf("C params: Yspike: %d, synapse: %f, dC: %f, p': %f\n", l->fired[ *ni],l->syn[ *ni ][ *syn_id ], dC, pstroke(u,c));
            exit(1);
        }

   }
   if(l->saveStat) {
        TEMPLATE(insertVector,double)(ls->stat_B[ *ni ], ls->B[ *ni ]);
        for(size_t con_i=0; con_i<l->nconn[ *ni ]; con_i++) {
            TEMPLATE(insertVector,double)(ls->stat_C[ *ni ][ con_i ], ls->C[ *ni ][ con_i ]);
        }
    }        
}
void resetValues_TOptimalSTDP(learn_t *ls_t, const size_t *ni) {
    TOptimalSTDP *ls = (TOptimalSTDP*)ls_t;
    SRMLayer *l = ls->base.l; 
    
    ls->B[ *ni ] = 0;
    for(size_t con_i=0; con_i < l->nconn[ *ni ]; con_i++) {
       ls->C[ *ni ][con_i] = 0;
    }
    indLNode *act_node = NULL;
    while( (act_node = TEMPLATE(getNextLList,ind)(ls->learn_syn_ids[ *ni ]) ) != NULL ) {
       TEMPLATE(dropNodeLList,ind)(ls->learn_syn_ids[*ni], act_node);
    }
}

void free_TOptimalSTDP(learn_t *ls_t) {
    TOptimalSTDP *ls = (TOptimalSTDP*)ls_t;
    SRMLayer *l = ls->base.l; 
    for(size_t ni=0; ni<l->N; ni++) {
        if(l->nconn[ni]>0) {
            free(ls->C[ni]);
        }
        TEMPLATE(deleteLList,ind)(ls->learn_syn_ids[ni]);
        if(l->saveStat) {
            TEMPLATE(deleteVector,double)(ls->stat_B[ni]);
            for(size_t con_i=0; con_i < l->nconn[ni]; con_i++) {
                TEMPLATE(deleteVector,double)(ls->stat_C[ni][con_i]);
            }                
            free(ls->stat_C[ni]);
        }
    }
    if(l->saveStat) {
        free(ls->stat_B);
        free(ls->stat_C);
    }

    free(ls->learn_syn_ids);
    free(ls->B);
    free(ls->C);
    free(ls);
}
