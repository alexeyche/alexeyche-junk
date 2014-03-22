#ifndef LAYER_H
#define LAYER_H

#include <stdlib.h>

#include "constants.h"
#include "core.h"

static int IDX = 0;

typedef enum {EXC, INH} nspec_t;

typedef struct {
    size_t N;
    size_t *ids;
    nspec_t *nt;

    double **W;
    double **syn;
    double **syn_spec;
    size_t **id_conns;
    int *nconn;
} SRMLayer;

SRMLayer* createSRMLayer(size_t N);
void printSRMLayer(SRMLayer *l);
void deleteSRMLayer(SRMLayer *l);
void configureSRMLayer(SRMLayer *l, Constants *c);

#endif
