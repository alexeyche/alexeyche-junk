#ifndef SIM_H
#define SIM_H

#include <layer.h>
#include <constants.h>
#include <spikes_list.h>
#include <net_sim.h>
#include <matrix.h>
#include <io.h>

typedef struct {
    indVector *input_spikes_iter;
    double t;
    double Tmax;
} SimRuntime;

typedef struct {
    pSRMLayerVector *layers;
    
    SimRuntime *rt;
    NetSim *ns;
} Sim;

Sim* createSim();
void appendLayerSim(Sim *s, SRMLayer *l);
void deleteSim(Sim *s);
void configureSim(Sim *s, Constants *c, bool saveStat);
SimRuntime* createRuntime();
void deleteRuntime(SimRuntime *sr);
void simulate(Sim *s, const Constants *c);
const SynSpike* getInputSpike(NetSim *ns, SimRuntime *sr, const size_t *n_id, const Constants *c);

#endif
