#ifndef SIM_H
#define SIM_H

#include <layer.h>
#include <constants.h>
#include <spikes_list.h>
#include <net_sim.h>
#include <matrix.h>
#include <io.h>
#include <pthread.h>

#define P( condition ) {if( (condition) != 0 ) { printf( "\n FAILURE in %s, line %d\n", __FILE__, __LINE__ );exit( 1 );}}

pthread_barrier_t barrier;

typedef struct {
    indVector *input_spikes_iter;
    indVector *spikes_iter;
    double t;
    double Tmax;
} SimRuntime;

typedef struct {
    const size_t *layer_id;
    const size_t *n_id;
} NeuronAddress;

typedef struct {
    pSRMLayerVector *layers;
    Constants *c;
    
    SimRuntime *rt;
    NetSim *ns;
    size_t net_size;
    
    size_t nthreads;
    NeuronAddress *na;
    size_t num_neurons;
} Sim;

typedef struct {
    Sim *s;
    int thread_id;
} SimWorker;

Sim* createSim();
void appendLayerSim(Sim *s, SRMLayer *l);
void deleteSim(Sim *s);
void configureLayersSim(Sim *s, Constants *c, bool saveStat);
SimRuntime* createRuntime();
void deleteRuntime(SimRuntime *sr);
void simulateNeuron(const double *t, const NeuronAddress *na, Sim *s, const Constants *c);
const SynSpike* getInputSpike(NetSim *ns, SimRuntime *sr, const size_t *n_id, const Constants *c);
void loadLayersFromFile(Sim *s, const char *model_fname, Constants *c, bool saveStat);
void saveLayersToFile(Sim *s, const char *model_file);
void configreNetSpikesSim(Sim *s, Constants *c);

void simRun(SimWorker *sw);

#endif
