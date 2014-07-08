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
#include <learn/learn.h>


#define SYN_ACT_TOL 0.0001 // value of synapse needed to delete 


typedef struct {
    unsigned char statLevel;
    doubleVector **stat_u;
    doubleVector **stat_p;
    doubleVector **stat_fired;
    doubleVector ***stat_W;
    doubleVector ***stat_syn;
} LayerStat;

typedef struct SynSpike {
    double t;
    size_t n_id;
    size_t syn_id;
} SynSpike;


struct SimContext;

typedef enum {EXC, INH} nspec_t;

struct learn_t;
struct LayerSerial;

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
    double *gr;
    double *M;

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

    // methods sim
    void (*calculateMembranePotentials)(struct Layer *l, const size_t *ni, const struct SimContext *s);
    void (*calculateSpike)(struct Layer *l, const size_t *ni, const struct SimContext *s);
    void (*calculateDynamics)(struct Layer *l, const size_t *ni, const struct SimContext *s);
    void (*propagateSpike)(struct Layer *l, const size_t *ni, const SynSpike *sp, const struct SimContext *s);
    // methods common    
    void (*deleteLayer)(struct Layer *l);
    void (*configureLayer)(struct Layer *l, const indVector *inputIDs, const indVector *outputIDs, const Constants *c);
    void (*toStartValues)(struct Layer *l, const Constants *c);
    void (*allocSynData)(struct Layer *l);
    void (*printLayer)(struct Layer *l);
    void (*serializeLayer)(struct Layer *l, FileStream *file, const Constants *c);
    void (*deserializeLayer)(struct Layer *l, FileStream *file, const Constants *c);
};
typedef struct Layer Layer;


Layer* createPoissonLayer(size_t N, size_t *glob_idx, unsigned char statLevel);

// methods sim
void calculateMembranePotentials_Poisson(Layer *l, const size_t *ni, const struct SimContext *s);
void calculateSpike_Poisson(Layer *l, const size_t *ni, const struct SimContext *s);
void calculateDynamics_Poisson(Layer *l, const size_t *ni, const struct SimContext *s);
void propagateSpike_Poisson(Layer *l, const size_t *ni, const SynSpike *sp, const struct SimContext *s);

// methods common
void toStartValues_Poisson(Layer *l, const Constants *c);
void allocSynData_Poisson(Layer *l);
void printLayer_Poisson(Layer *l);
void deleteLayer_Poisson(Layer *l);
void configureLayer_Poisson(Layer *l, const indVector *inputIDs, const indVector *outputIDs, const Constants *c);
void serializeLayer_Poisson(Layer *l, FileStream *file, const Constants *c);
void deserializeLayer_Poisson(Layer *l, FileStream *file, const Constants *c);

// common procedures
double layerConstD(Layer *l, doubleVector *v);
size_t getLocalNeuronId(Layer *l, const size_t *glob_id);
const size_t getGlobalId(Layer *l, const size_t *ni);
double getSynDelay(Layer *l, const size_t *ni, const size_t *syn_id);
void setSynapseSpeciality(Layer *l, size_t ni, size_t syn_id, double spec);

typedef Layer* pLayer;

#include <util/templates_clean.h>
#define T pLayer
#define DESTRUCT_METHOD deleteLayer
#include <util/util_vector_tmpl.h>
#endif
