#ifndef SIM_H
#define SIM_H

#include <stddef.h>

#include <layer.h>
#include <core/constants.h>
#include <core/util/spikes_list.h>
#include <core/util/matrix.h>
#include <core/util/io.h>

#include <pthread.h>

#ifndef max
    #define max( a, b ) ( ((a) > (b)) ? (a) : (b) )
#endif

#ifndef min
    #define min( a, b ) ( ((a) < (b)) ? (a) : (b) )
#endif
#define P( condition ) {if( (condition) != 0 ) { printf( "\n FAILURE in %s, line %d\n", __FILE__, __LINE__ );exit( 1 );}}

#include <core/sim/runtime.h>
#include <core/sim/netsim.h>




pthread_barrier_t barrier;

typedef struct {
    size_t layer_id;
    size_t n_id;
} NeuronAddress;

struct Sim {
    pSRMLayerVector *layers;

    double global_reward;
    double mean_global_reward;
    doubleVector *stat_global_reward;

    SimRuntime *rt;
    NetSim *ns;
    size_t net_size;
    
    size_t nthreads;
    NeuronAddress *na;
    size_t num_neurons;

    unsigned char stat_level;
};

typedef struct Sim Sim;

typedef struct {
    Sim *s;
    size_t thread_id;
} SimWorker;


NetSim* createNetSim();
void deallocNetSim(NetSim *ns);
void allocNetSim(NetSim *ns, size_t net_size);
void deleteNetSim(NetSim *ns);
void propagateInputSpikesNetSim(Sim *s, SpikesList *sl);
void printInputSpikesQueue(NetSim *ns);
void configureConnMapNetSim(NetSim *ns, pSRMLayerVector *l);
void propagateSpikeNetSim(Sim *s, SRMLayer *l, const size_t *ni, double t);


Sim* createSim(size_t nthreads, unsigned char stat_level, Constants *c);
void appendLayerSim(Sim *s, SRMLayer *l);
// configure
size_t getLayerIdOfNeuron(Sim *s, size_t n_id);
void deleteSim(Sim *s);
void configureLayersSim(Sim *s, Constants *c, unsigned char statLevel);
SimRuntime* createRuntime();
void deleteRuntime(SimRuntime *sr);
void configureNetSpikesSim(Sim *s, const char *input_spikes_filename, Constants *c);
void configureSimAttr(Sim *s);
void configureSynapses(Sim *s, Constants *c);

// serialize
void loadLayersFromFile(Sim *s, const char *model_fname, Constants *c, unsigned char statLevel);
void saveLayersToFile(Sim *s, const char *model_file);

// run
void runSim(Sim *s);
void simulateNeuron(Sim *s, const size_t *layer_id, const size_t *n_id, double t,  const Constants *c);
const SynSpike* getInputSpike(double t, const size_t *n_id, NetSim *ns, SimRuntime *sr, const Constants *c);
void* simRunRoutine(void *args);
void configureRewardModulation(Sim *s);


#endif
