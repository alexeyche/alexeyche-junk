#ifndef LAYER_H
#define LAYER_H

#include <stdlib.h>

static int IDX = 0;

typedef struct {
    size_t N;
    size_t *ids;

    double **W;
    double **syn;
    double **syn_spec;
    size_t **id_conns;
    int *nconn;
} SRMLayer;

SRMLayer* createSRMLayer(size_t N, size_t ninh);
void printSRMLayer(SRMLayer *l);
void deleteSRMLayer(SRMLayer *l);


#endif
