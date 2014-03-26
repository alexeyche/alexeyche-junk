
#include "layer.h"

#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#include <util/util.h>
#include <matrix.h>
#include <util/util_vector.h>

#include <templates_clean.h>
#define T pSRMLayer
#include <util/util_vector_tmpl.c>


SRMLayer* createSRMLayer(size_t N, size_t *glob_idx) {
    SRMLayer *l = (SRMLayer*)malloc(sizeof(SRMLayer));
    l->N = N;
    l->ids = (size_t*)malloc( l->N*sizeof(size_t));
    l->nt = (nspec_t*)malloc( l->N*sizeof(nspec_t));
    for(size_t ni=0; ni<l->N; ni++) {
        l->ids[ni] = ++(*glob_idx);
    }

    l->W = (double**)malloc( l->N*sizeof(double*));
    l->syn = (double**)malloc( l->N*sizeof(double*));
    l->syn_spec = (double**)malloc( l->N*sizeof(double*));
    l->id_conns = (size_t**)malloc( l->N*sizeof(size_t*));

    l->nconn = (int*)malloc( l->N*sizeof(int));
    for(size_t ni=0; ni<l->N; ni++) {
        l->nconn[ni] = 0;
    }
    return(l);
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

void deleteSRMLayer(SRMLayer *l) {
    for(size_t ni=0; ni<l->N; ni++) {
        if(l->nconn[ni]>0) {
            free(l->W[ni]);
            free(l->syn[ni]);
            free(l->syn_spec[ni]);
            free(l->id_conns[ni]);
        }
    }
    free(l->ids);
    free(l->nt);
    free(l->id_conns);
    free(l->nconn);
    free(l->W);
    free(l->syn);
    free(l->syn_spec);
    free(l);
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
        }
        TEMPLATE(deleteVector,ind)(conns);
    }
    for(size_t ni=0; ni<l->N; ni++) {
        double start_weight = c->weight_per_neuron/l->nconn[ni];
        for(size_t syn_i=0; syn_i<l->nconn[ni]; syn_i++) {
            l->W[ni][syn_i] = start_weight;
            l->syn[ni][syn_i] = 0;
            
            if(getSpecNeuron(l, &l->ids[ni]) == EXC) {
                l->syn_spec[ni][syn_i] = c->e_exc;
            } else 
            if(getSpecNeuron(l, &l->ids[ni]) == INH) {
                l->syn_spec[ni][syn_i] = -c->e_inh;
            }
        }
    }
}

