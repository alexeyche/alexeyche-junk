
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#include "layer.h"
#include "util/util.h"

SRMLayer* createSRMLayer(size_t N) {
    SRMLayer *l = (SRMLayer*)malloc(sizeof(SRMLayer));
    l->N = N;
    l->ids = (size_t*)malloc( l->N*sizeof(size_t));
    l->nt = (nspec_t*)malloc( l->N*sizeof(nspec_t));
    for(size_t ni=0; ni<l->N; ni++) {
        l->ids[ni] = IDX++;
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

void connectSRMLayer(Matrix *conn_m, indVector *ids_conn, Constants *c) {
    assert(conn_m->ncol == ids_conn->size);
    for(size_t i = 0; i<conn_m->nrow; i++) {
        for(size_t j = 0; j<conn_m->ncol; j++) {
            getMatrixElement(conn_m, i, j);
        } 
    }
}

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

void configureSRMLayer(SRMLayer *l, Constants *c) {
    srand(c->seed);
    for(size_t ni=0; ni<l->N; ni++) {
        indVector *conns = TEMPLATE(createVector,ind)(0);
        l->nt[ni] = EXC;
        if (c->inhib_frac > getUnif()) {
            l->nt[ni] = INH;
        }
        for(size_t nj=0; nj<l->N; nj++) {        
            if(ni != nj) {
                double r = getUnif();
                if(c->net_edge_prob > getUnif()) {
                    TEMPLATE(insertVector,ind)(conns, nj);
                }
            }
        }
        l->nconn[ni] = conns->size; 
        if(l->nconn[ni]>0) {
            l->id_conns[ni] = (size_t*) malloc(l->nconn[ni]*sizeof(size_t));
            memcpy(l->id_conns[ni], conns->array, l->nconn[ni]*sizeof(ind));
        }
        TEMPLATE(deleteVector,ind)(conns);
    }
}

