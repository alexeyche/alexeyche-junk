#ifndef SIM_H
#define SIM_H

#include <layer.h>
#include <constants.h>

typedef struct {
    double **net;
    size_t *net_lens;
    size_t net_size;    
} Net;

typedef struct {
    pSRMLayerVector *layers;
    
    Net *n;    
} Sim;

Sim* createSim();
void appendLayerSim(Sim *s, SRMLayer *l);
void deleteSim(Sim *s);

void configureSim(Sim *s, Constants *c);

#endif
