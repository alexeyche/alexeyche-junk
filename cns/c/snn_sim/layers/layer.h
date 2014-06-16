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
#include <util/util.h>
#include <util/matrix.h>

#include <neuron_funcs.h>
#include <util/io.h>
#include <learn.h>

struct LayerStat {
    unsigned char statLevel;
    doubleVector **stat_u;
    doubleVector **stat_p;
    doubleVector **stat_fired;
    doubleVector ***stat_W;
    doubleVector ***stat_syn;
};


struct Layer {
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
    double *u;
    double *p;
    double **syn;
    double **syn_spec;
    
    // learn structure 
    struct learn_t *ls_t;

    // runtime stuff
    indLList **active_syn_ids;
    unsigned char *fired;
    unsigned char **syn_fired;

    // delays
    double *axon_del;
    double **syn_del;

    // stat
    LayerStat *stat;

    // methods
    void (*calculateMembranePotentials)(struct Layer *l, const size_t *ni, const Constants *s, const double *t);
    void (*calculateSpike_Poisson)(struct Layer *l, const size_t *ni);
    void (*deleteLayer)(struct Layer *l);
    void (*configureLayer)(struct Layer *l, const indVector *inputIDs, const indVector *outputIDs, const Constants *c);
    void (*toStartValues)(struct Layer *l, const Constants *c);
    void (*propagateSpike)(struct Layer *l, const size_t *ni, const SynSpike *sp, const Constants *c);
    void (*allocSynData)(struct Layer *l);
    void (*printLayer)(struct Layer *l);
};

typedef struct Layer Layer;

Layer* createPoissonLayer(size_t N, size_t *glob_idx, unsigned char statLevel);
// common methods
void deleteLayer_Poisson(Layer *l);
void configureLayer_Poisson(Layer *l, const indVector *inputIDs, const indVector *outputIDs, const Constants *c);
void calculateMembranePotentials_Poisson(Layer *l, const size_t *ni, const Constants *s, const double *t);
void calculateSpike_Poisson(Layer *l, const size_t *ni);
void toStartValues_Poisson(Layer *l, const Constants *c);
void propagateSpike_Poisson(Layer *l, const size_t *ni, const SynSpike *sp, const Constants *c);
void allocSynData_Poisson(Layer *l);
void printLayer_Poisson(Layer *l);

// common procedures
double layerConstD(Layer *l, doubleVector *v);

#endif
