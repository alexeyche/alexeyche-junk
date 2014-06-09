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
struct Sim;

struct learn_t {
    struct SRMLayer *l;
    
    void (*toStartValues)(struct learn_t *);
    void (*propagateSynSpike)(struct learn_t *, const size_t *, const struct SynSpike *, const Constants *);
    void (*trainWeightsStep)(struct learn_t *, const double *, const double *, const double *, const size_t *, const struct Sim *);
    void (*resetValues)(struct learn_t *, const size_t *);
    void (*free)(struct learn_t *);
};
typedef struct learn_t learn_t;


#endif
