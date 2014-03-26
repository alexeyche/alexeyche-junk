#ifndef SIM_H
#define SIM_H

#include <layer.h>
#include <constants.h>

#include <spikes_list.h>

typedef struct {
    pSRMLayerVector *layers;
    
    SpikesList *net;    
} Sim;

Sim* createSim();
void appendLayerSim(Sim *s, SRMLayer *l);
void deleteSim(Sim *s);

void configureSim(Sim *s, Constants *c);

#endif
