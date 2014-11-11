#ifndef LAYER_H
#define LAYER_H

#include <stdlib.h>
#include <stddef.h>
#include <stdio.h>
#include <time.h>

#include <core/constants.h>
#include <core/core.h>

#include <core/util/util_vector.h>
#include <core/util/util_dlink_list.h>
#include <core/util/util.h>
#include <core/util/matrix.h>

#include <core/layers/neuron_funcs.h>
#include <core/util/io.h>
#include <learn.h>
#include <core/learn/optim.h>
#include <core/learn/res_stdp.h>

#define SYN_ACT_TOL 0.0001 // value of synapse needed to delete 

#define BACKPROP_POT 1
#define SFA 0
#define REFR 0
#define FS_INH 0

typedef enum {EXC, INH} nspec_t;

struct learn_t;

struct SRMLayer {
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

    struct learn_t *ls_t;
    
    indLList **active_syn_ids;
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
    unsigned char statLevel;
    doubleVector **stat_u;
    doubleVector **stat_p;
    doubleVector **stat_fired;
    doubleVector ***stat_W;
    doubleVector ***stat_syn;
};


typedef struct SRMLayer SRMLayer;
typedef SRMLayer* pSRMLayer;

#include <core/util/templates_clean.h>
#define T pSRMLayer
#define DESTRUCT deleteSRMLayer
#include <core/util/util_vector_tmpl.h>

typedef struct SynSpike {
    double t;
    size_t n_id;
    size_t syn_id;
} SynSpike;



SRMLayer* createSRMLayer(size_t N, size_t *glob_idx, unsigned char statLevel);
void printSRMLayer(SRMLayer *l);
void deleteSRMLayer(SRMLayer *l);
void configureSRMLayer(SRMLayer *l, const indVector *inputIDs, const indVector *outputIDs, Constants *c);

void simulateSRMLayerNeuron(SRMLayer *l, const size_t *id_to_sim, const struct Sim *s, const double *t);
pMatrixVector* serializeSRMLayer(SRMLayer *l);
void loadSRMLayer(SRMLayer *l, Constants *c, pMatrixVector *data);
void toStartValuesSRMLayer(SRMLayer *l, Constants *c);
void propagateSpikeSRMLayer(SRMLayer *l, const size_t *ni, const SynSpike *sp, const Constants *c);
void resetSRMLayerNeuron(SRMLayer *l, const size_t *ni);
double getSynDelay(SRMLayer *l, const size_t *id, const size_t *syn_id);
void setSynapseSpeciality(SRMLayer *l, size_t n_id, size_t syn_id, double spec);
size_t getLocalNeuronId(SRMLayer *l, const size_t *glob_id);
const size_t getGlobalId(SRMLayer *l, const size_t *ni);
double layerConstD(SRMLayer *l, doubleVector *v);

#endif
