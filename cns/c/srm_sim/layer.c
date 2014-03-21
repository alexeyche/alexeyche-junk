
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>

#include "layer.h"

SRMLayer* createSRMLayer(size_t N, Constants *c) {
    SRMLayer *l = (SRMLayer*)malloc(sizeof(SRMLayer));
    l->N = N;
    l->ids = (size_t*)malloc( l->N*sizeof(size_t));
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

void connectSRMLayer(Matrix *conn_m, indVector ids_conn, Constants *c) {
    assert(conn_m->ncol == ids_conn->size);
    for(size_t i = 0; i<conn_m->nrow; i++) {
        for(size_t j = 0; j<conn_m->ncol; j++) {
            getMatrixElement(conn_m, i, j);
        } 
    }
}

void printSRMLayer(SRMLayer *l) {
    printf("SRMLayer, size: %zu \n", l->N);
    printf("ids: ");
    for(size_t ni=0; ni<l->N; ni++) {
        printf("%zu,", l->ids[ni]);
    }
    printf("\n");
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
    free(l->id_conns);
    free(l->nconn);
    free(l->W);
    free(l->syn);
    free(l->syn_spec);
    free(l);
}
