
#include "layer.h"


#include <templates_clean.h>
#define T pSRMLayer
#define DESTRUCT deleteSRMLayer
#include <util/util_vector_tmpl.c>


SRMLayer* createSRMLayer(size_t N, size_t *glob_idx) {
    SRMLayer *l = (SRMLayer*)malloc(sizeof(SRMLayer));
    l->N = N;
    l->ids = (size_t*)malloc( l->N*sizeof(size_t));
    l->nt = (nspec_t*)malloc( l->N*sizeof(nspec_t));
    for(size_t ni=0; ni<l->N; ni++) {
        l->ids[ni] = (*glob_idx)++;
    }

    l->W = (double**)malloc( l->N*sizeof(double*));
    l->syn = (double**)malloc( l->N*sizeof(double*));
    l->syn_spec = (double**)malloc( l->N*sizeof(double*));
    l->id_conns = (size_t**)malloc( l->N*sizeof(size_t*));

    l->nconn = (int*)malloc( l->N*sizeof(int));
    for(size_t ni=0; ni<l->N; ni++) {
        l->nconn[ni] = 0;
    }
    l->active_syn_ids = TEMPLATE(createVector,ind)();
    l->a = (double*)malloc( l->N*sizeof(double));
    l->B = (double*)malloc( l->N*sizeof(double));
    l->C = (double**)malloc( l->N*sizeof(double*));
    l->fired = (unsigned char*)malloc( l->N*sizeof(unsigned char));
    l->pacc = (double*)malloc( l->N*sizeof(double));
    return(l);
}

void deleteSRMLayer(SRMLayer *l) {
    for(size_t ni=0; ni<l->N; ni++) {
        if(l->nconn[ni]>0) {
            free(l->W[ni]);
            free(l->syn[ni]);
            free(l->syn_spec[ni]);
            free(l->id_conns[ni]);
            free(l->C[ni]);
        }
    }
    free(l->ids);
    free(l->nt);
    free(l->id_conns);
    free(l->nconn);
    free(l->W);
    free(l->syn);
    free(l->syn_spec);
    TEMPLATE(deleteVector,ind)(l->active_syn_ids);
    free(l->a);
    free(l->B);
    free(l->C);
    free(l->fired);
    free(l->pacc);
    free(l);
}

//void connectSRMLayer(SRMLayer *l, Matrix *conn_m, indVector *ids_conn, Constants *c) {
//    assert(conn_m->ncol == ids_conn->size);
//    for(size_t i = 0; i<conn_m->nrow; i++) {
//        for(size_t j = 0; j<conn_m->ncol; j++) {
//            double wij = getMatrixElement(conn_m, i, j);
//        } 
//    }
//}

void printSRMLayer(SRMLayer *l) {
    printf("SRMLayer, size: %zu \n", l->N);
    for(size_t ni=0; ni<l->N; ni++) {
        printf("%zu, ", l->ids[ni]);
        if(l->nt[ni] == EXC) {
            printf("excitatory, ");
        } else 
        if(l->nt[ni] == INH) {
            printf("inhibitory, ");
        }
        printf("connected with: ");
        for(size_t syn_i=0; syn_i<l->nconn[ni]; syn_i++) {
            printf("%zu,", l->id_conns[ni][syn_i]);
        }
        printf("\n");
    }
}



nspec_t getSpecNeuron(SRMLayer *l, const size_t *id) {
    for(size_t ni=0; ni<l->N; ni++) {
        if( l->ids[ni] == *id ) {
            return(l->nt[ni]);
        }
    }
    printf("Error: Can't find neuron with id %zu\n", *id);
}

void configureSRMLayer(SRMLayer *l, const indVector *inputIDs, Constants *c) {
    srand(c->seed);
    for(size_t ni=0; ni<l->N; ni++) {
        indVector *conns = TEMPLATE(createVector,ind)(0);
        l->nt[ni] = EXC;
        if (c->inhib_frac > getUnif()) {
            l->nt[ni] = INH;
        }
        for(size_t nj=0; nj<l->N; nj++) {        
            if(ni != nj) {
                if(c->net_edge_prob > getUnif()) {
                    TEMPLATE(insertVector,ind)(conns, l->ids[nj]);
                }
            }
        }
        if(inputIDs) {
            for(size_t inp_i=0; inp_i<inputIDs->size; inp_i++) {        
                if(c->input_edge_prob > getUnif()) {
                    TEMPLATE(insertVector,ind)(conns, inputIDs->array[inp_i]);
                }
            }
        }
        l->nconn[ni] = conns->size; 
        if(l->nconn[ni]>0) {
            l->id_conns[ni] = (size_t*) malloc(l->nconn[ni]*sizeof(size_t));
            memcpy(l->id_conns[ni], conns->array, l->nconn[ni]*sizeof(size_t));
            l->W[ni] = (double*) malloc(l->nconn[ni]*sizeof(double));
            l->syn[ni] = (double*) malloc(l->nconn[ni]*sizeof(double));
            l->syn_spec[ni] = (double*) malloc(l->nconn[ni]*sizeof(double));
            l->C[ni] = (double*) malloc(l->nconn[ni]*sizeof(double));
        }
        TEMPLATE(deleteVector,ind)(conns);
    }
    for(size_t ni=0; ni<l->N; ni++) {
        double start_weight = c->weight_per_neuron/l->nconn[ni];
        l->a[ni] = 0;
        l->B[ni] = 0;
        l->fired[ni] = 0;
        l->pacc[ni] = 0;
        for(size_t syn_i=0; syn_i<l->nconn[ni]; syn_i++) {
            l->W[ni][syn_i] = start_weight;
            l->syn[ni][syn_i] = 0;
            l->C[ni][syn_i] = 0;

            if(getSpecNeuron(l, &l->ids[ni]) == EXC) {
                l->syn_spec[ni][syn_i] = c->e_exc;
            } else 
            if(getSpecNeuron(l, &l->ids[ni]) == INH) {
                l->syn_spec[ni][syn_i] = -c->e_inh;
            }
        }
    }
}

#define SYN_ACT_TOL 0.001

void simulateSRMLayerNeuron(SRMLayer *l, const size_t *id_to_sim, const Constants *c) {
    double u = 0;
    for(size_t act_i=0; act_i < l->active_syn_ids->size; act_i++) {
        const size_t *syn_id = &l->active_syn_ids->array[act_i];
        l->syn[ *id_to_sim ][ *syn_id ] -= l->syn[ *id_to_sim ][ *syn_id ]/c->tm;
        l->syn[ *id_to_sim ][ *syn_id ] *= l->a[ *id_to_sim ];
        u += l->W[ *id_to_sim ][ *syn_id ] * l->syn[ *id_to_sim ][ *syn_id ];
        if( l->syn[ *id_to_sim ][ *syn_id ] < SYN_ACT_TOL ) {
            TEMPLATE(removeVector,ind)(l->active_syn_ids, act_i);
        }
    }

    if(!c->determ) {
        double p = probf(&u, c) * c->dt;
        if( p > getUnif() ) {
            l->fired[ *id_to_sim ] = 1;
            l->pacc[ *id_to_sim ] += 1;
        }
        if(c->learn) {
            l->B[ *id_to_sim ] = B_calc( &l->fired[ *id_to_sim ], &p, &l->pacc[ *id_to_sim ], c);
            for(size_t act_i=0; act_i < l->active_syn_ids->size; act_i++) {
                const size_t *syn_id = &l->active_syn_ids->array[act_i];
                l->C[ *id_to_sim ][ *syn_id ] += l->C[ *id_to_sim ][ *syn_id ]/c->tc + \
                                                 C_calc( &l->fired[ *id_to_sim ], &p, &u, &l->syn[ *id_to_sim ][ *syn_id ], c);
/*TODO:*/       double dw = c->added_lrate*( l->C[ *id_to_sim ][ *syn_id ]*l->B[ *id_to_sim ] -  \
                                            c->weight_decay_factor * l->fired[ *id_to_sim ] * l->W[ *id_to_sim ][ *syn_id ] );
                l->W[ *id_to_sim ][ *syn_id ] += dw;
           }
        }
        l->pacc[ *id_to_sim ] -= l->pacc[ *id_to_sim ]/c->mean_p_dur; 
    } else { 
        if( u >= c->tr ) {
            l->fired[ *id_to_sim ] = 1;
        }
    }

}

