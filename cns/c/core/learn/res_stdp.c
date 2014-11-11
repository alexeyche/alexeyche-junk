

#include "res_stdp.h"
#include <core/layers/poisson.h>
#include <core/sim/sim.h>
#include <core/layers/neuron_funcs.h>

TResourceSTDP* init_TResourceSTDP(struct LayerPoisson *l) {
    TResourceSTDP *ls = (TResourceSTDP*) malloc( sizeof(TResourceSTDP) );    
    ls->base.l = l;
    ls->learn_syn_ids = (indLList**) malloc( l->N*sizeof(indLList*));
    ls->x_tr = (double**) malloc( l->N*sizeof(double*));
    ls->y_tr = (double*) malloc( l->N*sizeof(double));
    ls->res = (double*) malloc( l->N*sizeof(double));
    if(l->stat->statLevel>1) {
        ls->stat_x_tr = (doubleVector***) malloc( l->N * sizeof(doubleVector**) );
        ls->stat_y_tr = (doubleVector**) malloc( l->N * sizeof(doubleVector*) );
        ls->stat_res = (doubleVector**) malloc( l->N * sizeof(doubleVector*) );
    }
    ls->eligibility_trace = (double**) malloc( l->N * sizeof(double));
    ls->reward = (double*) malloc( l->N * sizeof(double));
    for(size_t ni=0; ni < l->N; ni++) {
        ls->eligibility_trace[ni] = (double*) malloc( l->nconn[ni] * sizeof(double) );
        ls->x_tr[ni] = (double*) malloc( l->nconn[ni] * sizeof(double) );
        ls->learn_syn_ids[ni] = TEMPLATE(createLList,ind)();
        if(l->stat->statLevel>1) {
            ls->stat_y_tr[ni] = TEMPLATE(createVector,double)();
            ls->stat_res[ni] = TEMPLATE(createVector,double)();
            ls->stat_x_tr[ni] = (doubleVector**) malloc( l->nconn[ni] * sizeof(doubleVector*) );
            for(size_t con_i=0; con_i < l->nconn[ni]; con_i++) {
                ls->stat_x_tr[ni][con_i] = TEMPLATE(createVector,double)();
            }
        }
    }
    ls->base.toStartValues = &toStartValues_TResourceSTDP;
    ls->base.propagateSynSpike = &propagateSynSpike_TResourceSTDP;
    ls->base.trainWeightsStep = &trainWeightsStep_TResourceSTDP;
    ls->base.resetValues = &resetValues_TResourceSTDP;
    ls->base.free = &free_TResourceSTDP;
    ls->base.serialize = &serialize_TResourceSTDP;
    ls->base.deserialize = &deserialize_TResourceSTDP;

    return(ls);
}

void toStartValues_TResourceSTDP(learn_t *ls_t) {
    TResourceSTDP *ls = (TResourceSTDP*)ls_t;
    LayerPoisson *l = ls->base.l;
    for(size_t ni=0; ni<l->N; ni++) {
        ls->y_tr[ni] = 0;
        ls->res[ni] = 1;
        ls->reward[ni] = 0.0;
        for(size_t con_i=0; con_i<l->nconn[ni]; con_i++) {
            ls->eligibility_trace[ni][con_i] = 0.0;
            ls->x_tr[ni][con_i] = 0;
        }
    }
}

void propagateSynSpike_TResourceSTDP(learn_t *ls_t, const size_t *ni, const struct SynSpike *sp, const Constants *c) {
    TResourceSTDP *ls = (TResourceSTDP*)ls_t;
    ls->x_tr[*ni][sp->syn_id] += 1;
    TEMPLATE(addValueLList,ind)(ls->learn_syn_ids[*ni], sp->syn_id);
}

double fastsqrt(double val) {
    long tmp = *(long *)&val;
    tmp -= 127L<<23; /* Remove IEEE bias from exponent (-2^23) */
    /* tmp is now an appoximation to logbase2(val) */
    tmp = tmp >> 1; /* divide by 2 */
    tmp += 127L<<23; /* restore the IEEE bias from the exponent (+2^23) */
    return *(double *)&tmp;
}


float fastsqrt2(float x) {
   return( 1+ (x-1)/2. - (1./8)*(x-1)*(x-1) - (1/16.)*(x-1)*(x-1)*(x-1) - (5/128.)*(x-1)*(x-1)*(x-1)*(x-1));
}

float Q_rsqrt( float number )
{
    long i;
    float x2, y;
    const float threehalfs = 1.5F;
 
    x2 = number * 0.5F;
    y  = number;
    i  = * ( long * ) &y;                       // evil floating point bit level hacking
    i  = 0x5f3759df - ( i >> 1 );               // what the fuck?
    y  = * ( float * ) &i;
    y  = y * ( threehalfs - ( x2 * y * y ) );   // 1st iteration
//      y  = y * ( threehalfs - ( x2 * y * y ) );   // 2nd iteration, this can be removed
 
    return 1.0/y;
}

void consumeResource(double *res, const double *dw, const Constants *c) {
    double dr = Q_rsqrt(fabs(*dw)*c->res_stdp->__Aplus_max_Amin);
    *res -= dr; 
    if(*res<0) {
        *res = 0;
    }
}

void trainWeightsStep_TResourceSTDP(learn_t *ls_t, const double *u, const double *p, const double *M, const size_t *ni, const SimContext *s) {
    TResourceSTDP *ls = (TResourceSTDP*)ls_t;
    LayerPoisson *l = ls->base.l;
    const Constants *c = s->c; 

    if(l->fired[*ni] == 1) {
        ls->y_tr[*ni] += 1;
    }

    indLNode *act_node = NULL;
    while( (act_node = TEMPLATE(getNextLList,ind)(ls->learn_syn_ids[ *ni ]) ) != NULL ) {
        const size_t *syn_id = &act_node->value;
        double dw = 0;
        if(l->fired[*ni] == 1) {
            dw += c->res_stdp->Aplus * ls->x_tr[*ni][*syn_id];
        }
        if(l->syn_fired[*ni][*syn_id] == 1) {
            dw -= c->res_stdp->Aminus * ls->y_tr[*ni];
        }
        double wmax = getLC(l,c)->wmax;
        dw = bound_grad(&l->W[*ni][*syn_id], &dw, &wmax, c);
        dw = dw * ls->res[*ni] * getLC(l,c)->lrate;
        l->W[*ni][*syn_id] += c->dt * dw;
        consumeResource(&ls->res[*ni], &dw, c);

        // melting
        ls->x_tr[*ni][*syn_id] += -c->dt * ls->x_tr[*ni][*syn_id] / c->res_stdp->tau_plus;
        if(c->reinforcement) {
            ls->eligibility_trace[*ni][*syn_id] += - c->dt * ls->eligibility_trace[*ni][*syn_id] / c->tel;
        }
        if((ls->x_tr[*ni][*syn_id] <= SYN_ACT_TOL)&&( (c->reinforcement)&&((ls->eligibility_trace[*ni][*syn_id]<=SYN_ACT_TOL)&&(ls->eligibility_trace[*ni][*syn_id]>=-SYN_ACT_TOL)))) {
            TEMPLATE(dropNodeLList,ind)(ls->learn_syn_ids[ *ni ], act_node);
        }
    }
    ls->y_tr[*ni] += -c->dt * ls->y_tr[*ni] / c->res_stdp->tau_minus;
    ls->res[*ni] += (1-ls->res[*ni])/c->res_stdp->tau_res;

    if(l->stat->statLevel>1) {
        TEMPLATE(insertVector,double)(ls->stat_y_tr[ *ni ], ls->y_tr[ *ni ]);
        TEMPLATE(insertVector,double)(ls->stat_res[ *ni ], ls->res[ *ni ]);
        for(size_t con_i=0; con_i<l->nconn[ *ni ]; con_i++) {
            TEMPLATE(insertVector,double)(ls->stat_x_tr[ *ni ][ con_i ], ls->x_tr[ *ni ][ con_i ]);
        }
    }        

}

void resetValues_TResourceSTDP(learn_t *ls_t, const size_t *ni) {
    TResourceSTDP *ls = (TResourceSTDP*)ls_t;
    LayerPoisson *l = ls->base.l;
    ls->y_tr[*ni] = 0;
    ls->res[*ni] = 1;
    for(size_t con_i=0; con_i<l->nconn[*ni]; con_i++) {
        ls->eligibility_trace[*ni][con_i] = 0.0;
        ls->reward[*ni] = 0.0;
        ls->x_tr[*ni][con_i] = 0;
    }
    indLNode *act_node = NULL;
    while( (act_node = TEMPLATE(getNextLList,ind)(ls->learn_syn_ids[ *ni ]) ) != NULL ) {
       TEMPLATE(dropNodeLList,ind)(ls->learn_syn_ids[*ni], act_node);
    }
}
void free_TResourceSTDP(learn_t *ls_t) {
    TResourceSTDP* ls = (TResourceSTDP*)ls_t;
    LayerPoisson *l = ls->base.l;
    for(size_t ni=0; ni < l->N; ni++) {
        if(l->nconn[ni]>0) {
            free(ls->x_tr[ni]);
        }
        TEMPLATE(deleteLList,ind)(ls->learn_syn_ids[ni]);
        if(l->stat->statLevel>1) {
            TEMPLATE(deleteVector,double)(ls->stat_y_tr[ni]);
            TEMPLATE(deleteVector,double)(ls->stat_res[ni]);
            for(size_t con_i=0; con_i < l->nconn[ni]; con_i++) {
                TEMPLATE(deleteVector,double)(ls->stat_x_tr[ni][con_i]);
            }
            free(ls->stat_x_tr[ni]);
        }
    }
    if(l->stat->statLevel>1) {
        free(ls->stat_x_tr);        
        free(ls->stat_y_tr);
        free(ls->stat_res);
    }
    free(ls->res);
    free(ls->y_tr);
    free(ls->x_tr);
}

void serialize_TResourceSTDP(learn_t *ls_t, FileStream *file, const Sim *s) {
}
void deserialize_TResourceSTDP(learn_t *ls_t, FileStream *file, const Sim *s) {
}

