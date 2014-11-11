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
#include <core/util/mem.h>

#include <core/layers/neuron_funcs.h>
#include <core/util/io.h>
#include <core/learn/learn.h>

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
struct Sim;

typedef enum {EXC, INH} nspec_t;

struct learn_t;


struct LayerPoisson {
    size_t id;
    bool need_steps_sync;
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
    double *gr;
    double *M;
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
    
    void (*calculateProbability)(struct LayerPoisson *l, const size_t *ni, const struct SimContext *s); 
    void (*calculateSpike)(struct LayerPoisson *l, const size_t *ni, const struct SimContext *s);  
    void (*calculateDynamics)(struct LayerPoisson *l, const size_t *ni, const struct SimContext *s); 
    void (*propagateSpike)(struct LayerPoisson *l, const size_t *ni, const SynSpike *sp, const struct SimContext *s); 
    void (*deleteLayer)(struct LayerPoisson *l);   
    void (*configureLayer)(struct LayerPoisson *l, const indVector *inputIDs, const indVector *outputIDs, const Constants *c); 
    void (*toStartValues)(struct LayerPoisson *l, const Constants *c); 
    void (*allocSynData)(struct LayerPoisson *l); 
    void (*deallocSynData)(struct LayerPoisson *l); 
    void (*printLayer)(struct LayerPoisson *l); 
    void (*serializeLayer)(struct LayerPoisson *l, FileStream *file, const struct Sim *s); 
    void (*deserializeLayer)(struct LayerPoisson *l, FileStream *file, const struct Sim *s); 
    void (*saveStat)(struct LayerPoisson *l, pMatrixVector *mv);

    double (*prob_fun)(const double *u, const Constants *c);
    double (*prob_fun_stroke)(const double *u, const Constants *c);
};
typedef struct LayerPoisson LayerPoisson;

typedef struct {
    neuron_layer_t neuron_type;
    learning_rule_t learning_rule;
} LayerSerializeInfo;

LayerPoisson* createPoissonLayer(size_t N, size_t *glob_idx, unsigned char statLevel);

// methods sim
void calculateProbability_Poisson(LayerPoisson *l, const size_t *ni, const struct SimContext *s);
void calculateSpike_Poisson(LayerPoisson *l, const size_t *ni, const struct SimContext *s);
void calculateDynamics_Poisson(LayerPoisson *l, const size_t *ni, const struct SimContext *s);
void propagateSpike_Poisson(LayerPoisson *l, const size_t *ni, const SynSpike *sp, const struct SimContext *s);

// methods common
void toStartValues_Poisson(LayerPoisson *l, const Constants *c);
void allocSynData_Poisson(LayerPoisson *l);
void deallocSynData_Poisson(LayerPoisson *l);
void printLayer_Poisson(LayerPoisson *l);
void deleteLayer_Poisson(LayerPoisson *l);
void configureLayer_Poisson(LayerPoisson *l, const indVector *inputIDs, const indVector *outputIDs, const Constants *c);
void serializeLayer_Poisson(LayerPoisson *l, FileStream *file, const struct Sim *s);
void deserializeLayer_Poisson(LayerPoisson *l, FileStream *file, const struct Sim *s);
void saveStat_Poisson(LayerPoisson *l, pMatrixVector *mv); 

// common procedures
size_t getLocalNeuronId(LayerPoisson *l, const size_t *glob_id);
const size_t getGlobalId(LayerPoisson *l, const size_t *ni);
double getSynDelay(LayerPoisson *l, const size_t *ni, const size_t *syn_id);
void setSynapseSpeciality(LayerPoisson *l, size_t ni, size_t syn_id, double spec);
void writeLayerSerializeInfo(LayerSerializeInfo info, FileStream *file);
LayerSerializeInfo readLayerSerializeInfo(FileStream *file);
const LayerConstants* getLC(LayerPoisson *l, const Constants *c);

typedef LayerPoisson* pLayer;

#include <core/util/templates_clean.h>
#define T pLayer
#define DESTRUCT_METHOD deleteLayer
#include <core/util/util_vector_tmpl.h>
#endif
