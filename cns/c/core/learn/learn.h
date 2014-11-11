#ifndef LEARN_H
#define LEARN_H


#define LEARN_ACT_TOL 0.00001 // value of synapse needed to delete 

#define RATE_NORM PRESYNAPTIC

#define PRESYNAPTIC 1
#define POSTSYNAPTIC 2

#include <core/layers/neuron_funcs.h>
#include <core/constants.h>
#include <core/util/util_dlink_list.h>
#include <core/util/matrix.h>
#include <core/util/io.h>

struct LayerPoisson;
struct SynSpike;
struct SimContext;
struct Sim;

struct learn_t {
    struct LayerPoisson *l;
    
    void (*toStartValues)(struct learn_t *);
    void (*propagateSynSpike)(struct learn_t *, const size_t *, const struct SynSpike *, const Constants *);
    void (*trainWeightsStep)(struct learn_t *, const double *, const double *, const double *, const size_t *, const struct SimContext *);
    void (*resetValues)(struct learn_t *, const size_t *);
    void (*free)(struct learn_t *);
    void (*serialize)(struct learn_t*, FileStream *file, const struct Sim *s);
    void (*deserialize)(struct learn_t*, FileStream *file, const struct Sim *s);
    void (*saveStat)(struct learn_t*, pMatrixVector *mv);
};
typedef struct learn_t learn_t;

struct learn_serial_t {
    struct LayerPoisson *l;
};
typedef struct learn_serial_t learn_serial_t;

#endif
