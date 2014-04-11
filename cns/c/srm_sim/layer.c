
#include "layer.h"


#include <templates_clean.h>
#define T pSRMLayer
#define DESTRUCT deleteSRMLayer
#include <util/util_vector_tmpl.c>


SRMLayer* createSRMLayer(size_t N, size_t *glob_idx, bool saveStat) {
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
    l->active_syn_ids = (indLList**) malloc( l->N*sizeof(indLList*));
    l->learn_syn_ids = (indLList**) malloc( l->N*sizeof(indLList*));
    for(size_t ni=0; ni<l->N; ni++) {
        l->active_syn_ids[ni] = TEMPLATE(createLList,ind)();
        l->learn_syn_ids[ni] = TEMPLATE(createLList,ind)();
    }
    
    l->a = (double*)malloc( l->N*sizeof(double));
    l->B = (double*)malloc( l->N*sizeof(double));
    l->C = (double**)malloc( l->N*sizeof(double*));
    l->axon_del = (double*)malloc( l->N*sizeof(double));
    l->fired = (unsigned char*)malloc( l->N*sizeof(unsigned char));
    l->pacc = (double*)malloc( l->N*sizeof(double));
    l->syn_fired = (unsigned char**)malloc( l->N*sizeof(unsigned char*));
    l->saveStat = saveStat;
    if(l->saveStat) {
        l->stat_u = (doubleVector**) malloc( l->N*sizeof(doubleVector*));
        l->stat_p = (doubleVector**) malloc( l->N*sizeof(doubleVector*));
        l->stat_B = (doubleVector**) malloc( l->N*sizeof(doubleVector*));
        l->stat_W = (doubleVector***) malloc( l->N*sizeof(doubleVector**));
        l->stat_C = (doubleVector***) malloc( l->N*sizeof(doubleVector**));
        for(size_t ni=0; ni<l->N; ni++) {
            l->stat_u[ni] = TEMPLATE(createVector,double)();
            l->stat_p[ni] = TEMPLATE(createVector,double)();
            l->stat_B[ni] = TEMPLATE(createVector,double)();
        }
    }
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
            free(l->syn_fired[ni]);
        }
        TEMPLATE(deleteLList,ind)(l->active_syn_ids[ni]);
        TEMPLATE(deleteLList,ind)(l->learn_syn_ids[ni]);
        if(l->saveStat) {
            TEMPLATE(deleteVector,double)(l->stat_u[ni]);
            TEMPLATE(deleteVector,double)(l->stat_p[ni]);
            TEMPLATE(deleteVector,double)(l->stat_B[ni]);
            for(size_t con_i=0; con_i < l->nconn[ni]; con_i++) {
                TEMPLATE(deleteVector,double)(l->stat_W[ni][con_i]);
                TEMPLATE(deleteVector,double)(l->stat_C[ni][con_i]);
            }
        }
    }
    free(l->ids);
    free(l->nt);
    free(l->id_conns);
    free(l->nconn);
    free(l->W);
    free(l->syn);
    free(l->syn_spec);
    free(l->active_syn_ids);
    free(l->learn_syn_ids);
    free(l->a);
    free(l->B);
    free(l->C);
    free(l->fired);
    free(l->syn_fired);
    free(l->pacc);
    free(l->axon_del);
    if(l->saveStat) {    
        free(l->stat_u);
        free(l->stat_p);
        free(l->stat_B);
        free(l->stat_C);
        free(l->stat_W);
    }
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
            printf("%zu:%f,", l->id_conns[ni][syn_i], l->W[ni][syn_i]);
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

void allocSynData(SRMLayer *l, size_t ni) {
    l->id_conns[ni] = (size_t*) malloc(l->nconn[ni]*sizeof(size_t));
    l->W[ni] = (double*) malloc(l->nconn[ni]*sizeof(double));
    l->syn[ni] = (double*) malloc(l->nconn[ni]*sizeof(double));
    l->syn_spec[ni] = (double*) malloc(l->nconn[ni]*sizeof(double));
    l->C[ni] = (double*) malloc(l->nconn[ni]*sizeof(double));
    l->syn_fired[ni] = (unsigned char*) malloc(l->nconn[ni]*sizeof(unsigned char));
    if(l->saveStat) {
        l->stat_W[ni] = (doubleVector**) malloc( l->nconn[ni]*sizeof(doubleVector*));
        l->stat_C[ni] = (doubleVector**) malloc( l->nconn[ni]*sizeof(doubleVector*));
        for(size_t syn_i=0; syn_i < l->nconn[ni]; syn_i++) {
            l->stat_W[ni][syn_i] = TEMPLATE(createVector,double)();
            l->stat_C[ni][syn_i] = TEMPLATE(createVector,double)();
        }
    }
}

void deallocSynData(SRMLayer *l, size_t ni) {
    free(l->id_conns[ni]);
    free(l->W[ni]);
    free(l->syn[ni]);
    free(l->syn_spec[ni]);
    free(l->C[ni]);
    free(l->syn_fired[ni]);
    if(l->saveStat) {
        for(size_t syn_i=0; syn_i < l->nconn[ni]; syn_i++) {
            TEMPLATE(deleteVector,double)(l->stat_W[ni][syn_i]);
            TEMPLATE(deleteVector,double)(l->stat_C[ni][syn_i]);
        }
        free(l->stat_W[ni]);
        free(l->stat_C[ni]);
    }
}

void toStartValues(SRMLayer *l, Constants *c) {
    for(size_t ni=0; ni<l->N; ni++) {
        double start_weight = c->weight_var * getNorm() + c->weight_per_neuron/l->nconn[ni];
        l->a[ni] = 1;
        l->B[ni] = 0;
        l->fired[ni] = 0;
        l->pacc[ni] = 0;
        l->axon_del[ni] = 0;
        for(size_t syn_i=0; syn_i<l->nconn[ni]; syn_i++) {
            l->W[ni][syn_i] = start_weight;
            l->syn[ni][syn_i] = 0;
            l->C[ni][syn_i] = 0;
            l->syn_fired[ni][syn_i] = 0;
            if(getSpecNeuron(l, &l->ids[ni]) == EXC) {
                l->syn_spec[ni][syn_i] = c->e_exc;
            } else 
            if(getSpecNeuron(l, &l->ids[ni]) == INH) {
                l->syn_spec[ni][syn_i] = -c->e_inh;
            }
        }
    }        
}


void configureSRMLayer(SRMLayer *l, const indVector *inputIDs, Constants *c) {
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
            allocSynData(l, ni);
            memcpy(l->id_conns[ni], conns->array, l->nconn[ni]*sizeof(size_t));
        }
        TEMPLATE(deleteVector,ind)(conns);
    }
    toStartValues(l, c); 
    if(c->axonal_delays_rate > 0) {
        for(size_t ni=0; ni<l->N; ni++) {
            l->axon_del[ni] = c->axonal_delays_gain*getExp(c->axonal_delays_rate);
        }
    }
}

void propagateSpikeSRMLayer(SRMLayer *l, const size_t *ni, const SynSpike *sp, const Constants *c) {
    if(l->syn[*ni][ sp->syn_id ] < SYN_ACT_TOL) {
        TEMPLATE(addValueLList,ind)(l->active_syn_ids[*ni], sp->syn_id);
    } 
    if( (l->C[ *ni ][ sp->syn_id ] < LEARN_ACT_TOL ) && (l->C[ *ni ][ sp->syn_id ] > -LEARN_ACT_TOL ) ) {
        TEMPLATE(addValueLList,ind)(l->learn_syn_ids[*ni], sp->syn_id);
    }
    
    l->syn[*ni][ sp->syn_id ] += l->syn_spec[*ni][ sp->syn_id ] * c->e0;
    l->syn[*ni][ sp->syn_id ] *= l->a[*ni];
    l->syn_fired[*ni][ sp->syn_id ] = 1;
}

#define RATE_NORM PRESYNAPTIC
void simulateSRMLayerNeuron(SRMLayer *l, const size_t *id_to_sim, const Constants *c) {
    double u = c->u_rest;

    indLNode *act_node;
    while( (act_node = TEMPLATE(getNextLList,ind)(l->active_syn_ids[ *id_to_sim ]) ) != NULL ) {
        const size_t *syn_id = &act_node->value;
        u += l->W[ *id_to_sim ][ *syn_id ] * l->syn[ *id_to_sim ][ *syn_id ];
    }
    if(!c->determ) {
        double p = probf(&u, c) * c->dt;
        double coin = getUnif();
        if( p > coin ) {
            l->fired[ *id_to_sim ] = 1;
            l->pacc[ *id_to_sim ] += 1;
            l->a[ *id_to_sim ] = 0;
//            printf("spike %zu! p: %f, pacc: %f\n", *id_to_sim, p, l->pacc[ *id_to_sim ]);
        }
        
        if(c->learn) {
            l->B[ *id_to_sim ] = B_calc( &l->fired[ *id_to_sim ], &p, &l->pacc[ *id_to_sim ], c);
            while( (act_node = TEMPLATE(getNextLList,ind)(l->learn_syn_ids[ *id_to_sim ]) ) != NULL ) {
//            for(size_t con_i=0; con_i < l->nconn[ *id_to_sim ]; con_i++) {
//                const size_t *syn_id = &con_i;
                const size_t *syn_id = &act_node->value;
//                if( (l->C[ *id_to_sim ][ *syn_id ] == 0) && (l->syn[ *id_to_sim ][ *syn_id ] == 0) ) continue;
                double dC = C_calc( &l->fired[ *id_to_sim ], &p, &u, &l->syn[ *id_to_sim ][ *syn_id ], c);
                l->C[ *id_to_sim ][ *syn_id ] += -l->C[ *id_to_sim ][ *syn_id ]/c->tc + dC;
//                printf("dC: %f C: %f, params: %d %f %f %f\n", dC, l->C[ *id_to_sim ][ *syn_id ], l->fired[ *id_to_sim ], p, u, l->syn[ *id_to_sim ][ *syn_id ]);
                double lrate = rate_calc(&l->W[ *id_to_sim ][ *syn_id ], c);
                
#if RATE_NORM == PRESYNAPTIC
                double dw = c->added_lrate*lrate*( l->C[ *id_to_sim ][ *syn_id ]*l->B[ *id_to_sim ] -  \
                                            c->weight_decay_factor * l->syn_fired[ *id_to_sim ][ *syn_id ] * l->W[ *id_to_sim ][ *syn_id ] );
#elif RATE_NORM == POSTSYNAPTIC                
                double dw = c->added_lrate*lrate*( l->C[ *id_to_sim ][ *syn_id ]*l->B[ *id_to_sim ] -  \
                                            c->weight_decay_factor * (l->fired[ *id_to_sim ] + l->syn_fired[ *id_to_sim ][ *syn_id ]) * l->W[ *id_to_sim ][ *syn_id ] );
#endif               
                l->W[ *id_to_sim ][ *syn_id ] += dw;
                
                if(l->saveStat) {
                    TEMPLATE(insertVector,double)(l->stat_W[ *id_to_sim ][ *syn_id ], l->W[ *id_to_sim ][ *syn_id ]);
                    TEMPLATE(insertVector,double)(l->stat_C[ *id_to_sim ][ *syn_id ], l->C[ *id_to_sim ][ *syn_id ]);
                }
                
                
                if( (l->C[ *id_to_sim ][ *syn_id ] < LEARN_ACT_TOL ) && (l->C[ *id_to_sim ][ *syn_id ] > -LEARN_ACT_TOL ) && 
                    (dC < LEARN_ACT_TOL ) && (dC > -LEARN_ACT_TOL ) ) {

                    TEMPLATE(dropNodeLList,ind)(l->learn_syn_ids[ *id_to_sim ], act_node);
                }
                if( isnan(dw) ) { 
                    printf("\nFound bad value\n");
                    printf("nid: %zu, p: %f, u: %f, B: %f, pacc: %f, C: %f, lrate: %f, W: %f, dw: %f\n", *id_to_sim, p, u, l->B[ *id_to_sim ], l->pacc[ *id_to_sim ], l->C[ *id_to_sim ][ *syn_id ], lrate, l->W[ *id_to_sim ][ *syn_id ], dw);
                    printf("C params: Yspike: %d, synapse: %f, dC: %f, p': %f\n", l->fired[ *id_to_sim],l->syn[ *id_to_sim ][ *syn_id ], dC, pstroke(&u,c));
                    exit(1);
                }
 
           }
        }

        l->pacc[ *id_to_sim ] -= l->pacc[ *id_to_sim ]/c->mean_p_dur; 

        if(l->saveStat) {
            TEMPLATE(insertVector,double)(l->stat_p[ *id_to_sim ], p);
            TEMPLATE(insertVector,double)(l->stat_u[ *id_to_sim ], u);
            TEMPLATE(insertVector,double)(l->stat_B[ *id_to_sim ], l->B[ *id_to_sim ]);
        }

    } else { 
        if( u >= c->tr ) {
            l->fired[ *id_to_sim ] = 1;
            l->a[ *id_to_sim ] = 0;
        }
        if(l->saveStat) {
            TEMPLATE(insertVector,double)(l->stat_u[ *id_to_sim ], u);
        }
    }

    while( (act_node = TEMPLATE(getNextLList,ind)(l->active_syn_ids[ *id_to_sim ]) ) != NULL ) {
        const size_t *syn_id = &act_node->value;
        l->syn[ *id_to_sim ][ *syn_id ] -= l->syn[ *id_to_sim ][ *syn_id ]/c->tm;
        if(l->syn_fired[ *id_to_sim ][ *syn_id ] == 1) {
            l->syn[ *id_to_sim ][ *syn_id ] *= l->a[*id_to_sim];
            l->syn_fired[ *id_to_sim ][ *syn_id ] = 0; 
        }
        if( l->syn[ *id_to_sim ][ *syn_id ] < SYN_ACT_TOL ) {
            TEMPLATE(dropNodeLList,ind)(l->active_syn_ids[ *id_to_sim ], act_node);
        }
    }
    l->a[ *id_to_sim ] += (1 - l->a[ *id_to_sim ])/c->ta;
}


void resetSRMLayerNeuron(SRMLayer *l, const size_t *ni) {
    l->a[ *ni ] = 1;
    l->B[ *ni ] = 0;
    l->fired[ *ni ] = 0;
    for(size_t con_i=0; con_i < l->nconn[ *ni ]; con_i++) {
        l->C[ *ni ][con_i] = 0;
        l->syn[ *ni ][con_i] = 0;
        l->syn_fired[ *ni ][con_i] = 0;
    }
    indLNode *act_node = NULL;
    while( (act_node = TEMPLATE(getNextLList,ind)(l->active_syn_ids[ *ni ]) ) != NULL ) {
       TEMPLATE(dropNodeLList,ind)(l->active_syn_ids[ *ni], act_node);
    }
    act_node = NULL;
    while( (act_node = TEMPLATE(getNextLList,ind)(l->learn_syn_ids[ *ni ]) ) != NULL ) {
       TEMPLATE(dropNodeLList,ind)(l->learn_syn_ids[*ni], act_node);
    }
}

pMatrixVector* serializeSRMLayer(SRMLayer *l) {
    pMatrixVector *data = TEMPLATE(createVector,pMatrix)();
    size_t max_conn_id = 0;
    for(size_t ni=0; ni< l->N; ni++) {
        for(size_t con_i=0; con_i < l->nconn[ni]; con_i++) {
            if(max_conn_id < l->id_conns[ni][con_i]) {
                max_conn_id = l->id_conns[ni][con_i];
            }
        }

    }
    Matrix *W = createMatrix(l->N, max_conn_id+1);
    Matrix *nt = createMatrix(l->N, 1);
    Matrix *pacc = createMatrix(l->N, 1);
    Matrix *ids = createMatrix(l->N, 1);
    Matrix *axon_del = createMatrix(l->N, 1);
   
    for(size_t i=0; i<W->nrow; i++) {
        for(size_t j=0; j<W->ncol; j++) {
            setMatrixElement(W, i, j, 0);
        }
    }
    for(size_t ni=0; ni< l->N; ni++) {
        for(size_t con_i=0; con_i < l->nconn[ni]; con_i++) {
            setMatrixElement(W, ni, l->id_conns[ni][con_i], l->W[ni][con_i]);
        }
        setMatrixElement(nt, ni, 0, l->nt[ni]);
        setMatrixElement(pacc, ni, 0, l->pacc[ni]);
        setMatrixElement(ids, ni, 0, l->ids[ni]);
        setMatrixElement(axon_del, ni, 0, l->axon_del[ni]);
    }
    
    TEMPLATE(insertVector,pMatrix)(data, W);
    TEMPLATE(insertVector,pMatrix)(data, nt);
    TEMPLATE(insertVector,pMatrix)(data, pacc);
    TEMPLATE(insertVector,pMatrix)(data, ids);
    TEMPLATE(insertVector,pMatrix)(data, axon_del);
   
    return(data);
}

void loadSRMLayer(SRMLayer *l, Constants *c, pMatrixVector *data) {
    Matrix *W = data->array[0];
    Matrix *nt = data->array[1];
    Matrix *pacc = data->array[2];
    Matrix *ids = data->array[3];
    Matrix *axon_del = data->array[4];
    assert(l->N == W->nrow);

    doubleVector **W_vals = (doubleVector**) malloc( W->nrow * sizeof(doubleVector*));
    indVector **id_conns_vals = (indVector**) malloc( W->nrow * sizeof(indVector*));
    for(size_t i=0; i<W->nrow; i++) {  // shape form and get the values
        W_vals[i] = TEMPLATE(createVector,double)();
        id_conns_vals[i] = TEMPLATE(createVector,ind)();
        for(size_t j=0; j<W->ncol; j++) {
            double w_ij = getMatrixElement(W, i, j);
            if(w_ij>0) {
                TEMPLATE(insertVector,double)(W_vals[i], w_ij);
                TEMPLATE(insertVector,ind)(id_conns_vals[i], j);
            }
        }
        if(l->nconn[i] > 0) {
            deallocSynData(l, i);
        }
        l->nconn[i] = W_vals[i]->size;
        allocSynData(l, i);
        if(getMatrixElement(nt, i, 0) == 0) {
            l->nt[i] = EXC;
        } else 
        if(getMatrixElement(nt, i, 0) == 1.0) {
            l->nt[i] = INH;
        } else {
            printf("Error in neuron type\n");
            exit(1);
        }
        l->ids[i] = getMatrixElement(ids, i, 0);    
    }
    toStartValues(l, c);
    for(size_t ni=0; ni<l->N; ni++) {  // apply values
        memcpy(l->W[ni], W_vals[ni]->array, sizeof(double)*W_vals[ni]->size);
        memcpy(l->id_conns[ni], id_conns_vals[ni]->array, sizeof(size_t)*id_conns_vals[ni]->size);
        l->pacc[ni] = getMatrixElement(pacc, ni, 0);
        l->axon_del[ni] = getMatrixElement(axon_del, ni, 0);

        TEMPLATE(deleteVector,double)(W_vals[ni]);
        TEMPLATE(deleteVector,ind)(id_conns_vals[ni]);
    }
    free(W_vals);
    free(id_conns_vals);
}

