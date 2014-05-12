#ifndef LEARN_H
#define LEARN_H


#define LEARN_ACT_TOL 0.00001 // value of synapse needed to delete 

#define RATE_NORM PRESYNAPTIC

#define PRESYNAPTIC 1
#define POSTSYNAPTIC 2

#include <neuron_funcs.h>
#include <constants.h>
#include <util/util_dlink_list.h>

struct SRMLayer;
struct SynSpike;

struct learn_t {
    struct SRMLayer *l;
    
    void (*toStartValues)(struct learn_t *);
    void (*propagateSynSpike)(struct learn_t *, const size_t *, const struct SynSpike *, const Constants *);
    void (*trainWeightsStep)(struct learn_t *, const double *, const double *, const double *, const size_t *, const Constants *);
    void (*resetValues)(struct learn_t *, const size_t *);
    void (*free)(struct learn_t *);
};
typedef struct learn_t learn_t;


<<<<<<< HEAD
=======
typedef struct {
    learn_t base;
    
    indLList **learn_syn_ids;
    double **C;
    double *B;

    doubleVector **stat_B;
    doubleVector ***stat_C;
} TOptimalSTDP;

TOptimalSTDP* init_TOptimalSTDP(struct SRMLayer *l);
void toStartValues_TOptimalSTDP(learn_t *ls_t);
void propagateSynSpike_TOptimalSTDP(learn_t *ls_t, const size_t *ni, const struct SynSpike *sp, const Constants *c);
void trainWeightsStep_TOptimalSTDP(learn_t *ls_t, const double *u, const double *p, const double *M, const size_t *ni, const Constants *c);
void resetValues_TOptimalSTDP(learn_t *ls_t, const size_t *ni);
void free_TOptimalSTDP(learn_t *ls_t);

>>>>>>> 318e8ea01391cb34992d34a456f378d5952f3290
#endif
