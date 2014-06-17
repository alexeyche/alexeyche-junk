#ifndef SIM_H
#define SIM_H

#include <stddef.h>

#include <layer.h>
#include <constants.h>
#include <util/spikes_list.h>
#include <util/matrix.h>
#include <util/io.h>

#include <pthread.h>

#ifndef max
    #define max( a, b ) ( ((a) > (b)) ? (a) : (b) )
#endif

#ifndef min
    #define min( a, b ) ( ((a) < (b)) ? (a) : (b) )
#endif
#define P( condition ) {if( (condition) != 0 ) { printf( "\n FAILURE in %s, line %d\n", __FILE__, __LINE__ );exit( 1 );}}

#include <sim/runtime.h>
#include <sim/netsim.h>


pthread_barrier_t barrier;

typedef struct {
    double global_reward;
    double mean_global_reward;
    doubleVector *stat_global_reward;
    double t
    const Constants *c;
    unsigned char stat_level;
} SimContext;


typedef struct {
    size_t layer_id;
    size_t n_id;
} NeuronAddress;

struct SimImpl {
    size_t net_size;
    size_t nthreads;
    NeuronAddress *na;
    size_t num_neurons;
};

typedef struct {
    Sim *s;
    size_t thread_id;
} SimWorker;

typedef struct Sim {
    pSRMLayerVector *layers;
    NetSim *ns;
    SimRuntime *rt;
    SimContext *ctx;    

    SimImpl *impl
} Sim;


Sim* createSim(size_t nthreads, unsigned char stat_level, Constants *c);
void appendLayerSim(Sim *s, SRMLayer *l);
void deleteSim(Sim *s);
size_t getLayerIdOfNeuron(Sim *s, size_t n_id);

void runSim(Sim *s);
void simulateNeuron(Sim *s, const size_t *layer_id, const size_t *n_id, double t,  const Constants *c);
const SynSpike* getInputSpike(double t, const size_t *n_id, NetSim *ns, SimRuntime *sr, const Constants *c);
void* simRunRoutine(void *args);


#endif
