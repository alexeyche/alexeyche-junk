#ifndef LAYER_H
#define LAYER_H

#include <stdlib.h>
#include <stddef.h>
#include <stdio.h>
#include <time.h>



#include <constants.h>
#include <core.h>
#include <util/util_vector.h>
#include <neuron_funcs.h>
#include <util/util.h>


typedef enum {EXC, INH} nspec_t;

typedef struct {
    //consts
    size_t N;
    size_t *ids;
    nspec_t *nt;

    // structure
    double **W;
    size_t **id_conns;
    int *nconn;
    
    // vars 
    double **syn;
    double **syn_spec;
    double *a;
    double **C;
    double *B;
    indVector *active_syn_ids;
    unsigned char *fired;
    double *pacc;
} SRMLayer;

typedef SRMLayer* pSRMLayer;

#include <templates_clean.h>
#define T pSRMLayer
#define DESTRUCT deleteSRMLayer
#include <util/util_vector_tmpl.h>


SRMLayer* createSRMLayer(size_t N, size_t *glob_idx);
void printSRMLayer(SRMLayer *l);
void deleteSRMLayer(SRMLayer *l);
void configureSRMLayer(SRMLayer *l, const indVector *inputIDs, Constants *c);
nspec_t getSpecNeuron(SRMLayer *l, const size_t *id);

void simulateSRMLayerNeuron(SRMLayer *l, const size_t *id_to_sim, const Constants *c);


#endif
