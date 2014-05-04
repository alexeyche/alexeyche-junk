#ifndef LAYER_H
#define LAYER_H

#include <stdlib.h>
#include <stddef.h>
#include <stdio.h>
#include <time.h>

#include <constants.h>
#include <core.h>
#include <util/util_vector.h>
#include <util/util_dlink_list.h>
#include <neuron_funcs.h>
#include <util/util.h>
#include <matrix.h>
#include <io.h>

#define SYN_ACT_TOL 0.0001 // value of synapse needed to delete 
#define LEARN_ACT_TOL 0.00001 // value of synapse needed to delete 

#define PRESYNAPTIC 1
#define POSTSYNAPTIC 2

#define RATE_NORM PRESYNAPTIC
#define BACKPROP_POT 0
#define SFA 1
#define REFR 1
#define FS_INH 0

typedef enum {EXC, INH} nspec_t;

typedef struct {
    size_t id;
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
    indLList **active_syn_ids;
    indLList **learn_syn_ids;
    unsigned char *fired;
    unsigned char **syn_fired;
    double *pacc;
    double *axon_del;
    double **syn_del;
    double *gr;
    double *ga;
    double *gb;
    
    double *p_set;
    //stat
    bool saveStat;
    doubleVector **stat_B;
    doubleVector **stat_u;
    doubleVector **stat_p;
    doubleVector ***stat_W;
    doubleVector ***stat_C;
} SRMLayer;

typedef SRMLayer* pSRMLayer;

#include <templates_clean.h>
#define T pSRMLayer
#define DESTRUCT deleteSRMLayer
#include <util/util_vector_tmpl.h>

typedef struct {
    double t;
    size_t n_id;
    size_t syn_id;
} SynSpike;


SRMLayer* createSRMLayer(size_t N, size_t *glob_idx, bool saveStat);
void printSRMLayer(SRMLayer *l);
void deleteSRMLayer(SRMLayer *l);
void configureSRMLayer(SRMLayer *l, const indVector *inputIDs, const indVector *outputIDs, Constants *c);

void simulateSRMLayerNeuron(SRMLayer *l, const size_t *id_to_sim, const Constants *c);
pMatrixVector* serializeSRMLayer(SRMLayer *l);
void loadSRMLayer(SRMLayer *l, Constants *c, pMatrixVector *data);
void toStartValues(SRMLayer *l, Constants *c);
void propagateSpikeSRMLayer(SRMLayer *l, const size_t *ni, const SynSpike *sp, const Constants *c);
void resetSRMLayerNeuron(SRMLayer *l, const size_t *ni);
double getSynDelay(SRMLayer *l, const size_t *id, const size_t *syn_id);
void setSynapseSpeciality(SRMLayer *l, size_t n_id, size_t syn_id, double spec);
size_t getLocalNeuronId(SRMLayer *l, const size_t *glob_id);
const size_t getGlobalId(SRMLayer *l, const size_t *ni);

#endif
