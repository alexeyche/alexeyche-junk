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
} SimRuntime;

typedef struct {
    pSRMLayerVector *layers;
    
    SimRuntime *rt;
    NetSim *ns;
} Sim;

Sim* createSim();
void appendLayerSim(Sim *s, SRMLayer *l);
void deleteSim(Sim *s);
void configureSim(Sim *s, Constants *c);
SimRuntime* createRuntime();
void deleteRuntime(SimRuntime *sr);


#endif
