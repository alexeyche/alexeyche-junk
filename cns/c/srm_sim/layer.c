
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
    l->active_syn_ids = (indVector**) malloc( l->N*sizeof(indVector*));
    for(size_t ni=0; ni<l->N; ni++) {
        l->active_syn_ids[ni] = TEMPLATE(createVector,ind)();
    }
    
    l->a = (double*)malloc( l->N*sizeof(double));
    l->B = (double*)malloc( l->N*sizeof(double));
    l->C = (double**)malloc( l->N*sizeof(double*));
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
            printf("ni == %zu\n", ni);
            free(l->W[ni]);
            free(l->syn[ni]);
            free(l->syn_spec[ni]);
            free(l->id_conns[ni]);
            free(l->C[ni]);
            free(l->syn_fired[ni]);
        }
        if(l->saveStat) {
            TEMPLATE(deleteVector,double)(l->stat_u[ni]);
            TEMPLATE(deleteVector,double)(l->stat_p[ni]);
            TEMPLATE(deleteVector,double)(l->stat_B[ni]);
            TEMPLATE(deleteVector,ind)(l->active_syn_ids[ni]);
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
    free(l->a);
    free(l->B);
    free(l->C);
    free(l->fired);
    free(l->syn_fired);
    free(l->pacc);
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
            l->syn_fired[ni] = (unsigned char*) malloc(l->nconn[ni]*sizeof(unsigned char));
        }
        TEMPLATE(deleteVector,ind)(conns);
    }
    for(size_t ni=0; ni<l->N; ni++) {
        double start_weight = c->weight_per_neuron/l->nconn[ni];
        l->a[ni] = 1;
        l->B[ni] = 0;
        l->fired[ni] = 0;
        l->pacc[ni] = 0;
        if(l->saveStat) {
            l->stat_W[ni] = (doubleVector**) malloc( l->nconn[ni]*sizeof(doubleVector*));
            l->stat_C[ni] = (doubleVector**) malloc( l->nconn[ni]*sizeof(doubleVector*));
        }
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
            if(l->saveStat) {
                l->stat_W[ni][syn_i] = TEMPLATE(createVector,double)();
                l->stat_C[ni][syn_i] = TEMPLATE(createVector,double)();
            }
        }
    }
}


void simulateSRMLayerNeuron(SRMLayer *l, const size_t *id_to_sim, const Constants *c) {
    assert( *id_to_sim < l->N);
    double u = c->u_rest;
//    printf("active synapse0: ");
    for(size_t act_i=0; act_i < l->active_syn_ids[ *id_to_sim ]->size; act_i++) {
//        printf("act size: %zu\n", l->active_syn_ids[ni]->size);
        const size_t *syn_id = &l->active_syn_ids[ *id_to_sim ]->array[act_i];
        l->syn[ *id_to_sim ][ *syn_id ] -= l->syn[ *id_to_sim ][ *syn_id ]/c->tm;
        l->syn[ *id_to_sim ][ *syn_id ] *= l->a[ *id_to_sim ];
        
        u += l->W[ *id_to_sim ][ *syn_id ] * l->syn[ *id_to_sim ][ *syn_id ];
//        printf("id: %zu  w: %f  act: %f |", *syn_id, l->W[ *id_to_sim ][ *syn_id ], l->syn[ *id_to_sim ][ *syn_id ]);
        if( l->syn[ *id_to_sim ][ *syn_id ] < SYN_ACT_TOL ) {
            TEMPLATE(removeVector,ind)(l->active_syn_ids[ *id_to_sim ], act_i);
        }
    }
//    printf("\n");
//    printf("u: %f\n", u);
    l->a[ *id_to_sim ] += (1 - l->a[ *id_to_sim ])/c->ta;
    if(!c->determ) {
        double p = probf(&u, c) * c->dt;
        double coin = getUnif();
        if( p > coin ) {
            printf("%f > %f\n", p, coin);
            l->fired[ *id_to_sim ] = 1;
            l->pacc[ *id_to_sim ] += 1;
            l->a[ *id_to_sim ] = 0;
        }
//        else printf("%f <= %f\n", p, coin);
        
        if(c->learn) {
            l->B[ *id_to_sim ] = B_calc( &l->fired[ *id_to_sim ], &p, &l->pacc[ *id_to_sim ], c);
//            printf("active synapse1: ");
            for(size_t act_i=0; act_i < l->active_syn_ids[ *id_to_sim ]->size; act_i++) {
                const size_t *syn_id = &l->active_syn_ids[ *id_to_sim ]->array[act_i];
                assert( *syn_id < l->nconn[ *id_to_sim ]);
                l->C[ *id_to_sim ][ *syn_id ] += C_calc( &l->fired[ *id_to_sim ], &p, &u, &l->syn[ *id_to_sim ][ *syn_id ], c);
/*TODO:*/       double dw = c->added_lrate*rate_calc(&l->W[ *id_to_sim ][ *syn_id ])*( l->C[ *id_to_sim ][ *syn_id ]*l->B[ *id_to_sim ] -  \
                                            c->weight_decay_factor * l->syn_fired[ *id_to_sim ][ *syn_id ] * l->W[ *id_to_sim ][ *syn_id ] );
                
//                printf("id: %zu  w: %f  act: %f dw: %f C: %f|", *syn_id, l->W[ *id_to_sim ][ *syn_id ], l->syn[ *id_to_sim ][ *syn_id ], dw, l->C[ *id_to_sim ][ *syn_id ]);
                if( isnan(dw) ) {
                    printf("\nFound nan value\n");
                    printf("nid: %zu, p: %f, u: %f, B: %f, pacc: %f\n", *id_to_sim, p, u, l->B[ *id_to_sim ], l->pacc[ *id_to_sim ]);
                    exit(1);
                }
                l->W[ *id_to_sim ][ *syn_id ] += dw;
                l->syn_fired[ *id_to_sim ][ *syn_id ] = 0; // not a good place for that but this is convinient
           }
//           printf("\n");
        }
        for(size_t con_i=0; con_i < l->nconn[ *id_to_sim ]; con_i++) {
            if(l->saveStat) {
                TEMPLATE(insertVector,double)(l->stat_W[ *id_to_sim ][ con_i ], l->W[ *id_to_sim ][ con_i ]);
                TEMPLATE(insertVector,double)(l->stat_C[ *id_to_sim ][ con_i ], l->C[ *id_to_sim ][ con_i ]);
            }
            l->C[ *id_to_sim ][ con_i ] += -l->C[ *id_to_sim ][ con_i ]/c->tc;
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
        }
    }
}

void serializeSRMLayer(SRMLayer *l, const char *filename) {
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
    
    for(size_t i=0; i<W->nrow; i++) {
        for(size_t j=0; j<W->ncol; j++) {
            setMatrixElement(W, i, j, 0);
        }
    }
    for(size_t ni=0; ni< l->N; ni++) {
        for(size_t con_i=0; con_i < l->nconn[ni]; con_i++) {
            setMatrixElement(W, ni, l->id_conns[ni][con_i], l->W[ni][con_i]);
            setMatrixElement(syn_spec, ni, l->id_conns[ni][con_i], l->syn_spec[ni][con_i]);
        }
        setMatrixElement(nt, ni, 1, l->nt[ni]);
        setMatrixElement(pacc, ni, 1, l->pacc[ni]);
    }
    
    TEMPLATE(insertVector,pMatrix)(data, W);
    TEMPLATE(insertVector,pMatrix)(data, nt);
    TEMPLATE(insertVector,pMatrix)(data, pacc);

    saveMatrixList(filename, data);
    
    TEMPLATE(deleteVector,pMatrix)(data);
}

void loadSRMLayerFromFile(SRMLayer *l, const char *model_file) {
    pMatrixVector* data = readMatrixList(model_file);
    Matrix *W = data->a
}

