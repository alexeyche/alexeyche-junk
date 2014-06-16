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
    void (*calculateMembranePotentials)(Layer *l, const size_t *ni, const Constants *s, const double *t);
    void (*calculateSpike_default)(Layer *l, const size_t *ni);
};

typedef struct Layer Layer;

// common methods
Layer* createLayer(size_t N, size_t *glob_idx, unsigned char statLevel);
void deleteLayer(Layer *l);
void configureLayer(Layer *l, const indVector *inputIDs, const indVector *outputIDs, Constants *c);

// default virtuals
void calculateMembranePotentials_default(Layer *l, const size_t *ni, const Constants *s, const double *t);
void calculateSpike_default(Layer *l, const size_t *ni);


#endif
