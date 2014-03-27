#ifndef SIM_H
#define SIM_H

#include <layer.h>
#include <constants.h>
#include <spikes_list.h>
#include <net_sim.h>

typedef struct {
    pSRMLayerVector *layers;
    
    NetSim *ns;
} Sim;

Sim* createSim();
void appendLayerSim(Sim *s, SRMLayer *l);
void deleteSim(Sim *s);
void configureSim(Sim *s, Constants *c);

#endif
