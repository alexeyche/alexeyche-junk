#ifndef LAYER_SYN_BACKPROP_H
#define LAYER_SYN_BACKPROP_H

typedef struct {
    SRMLayer base;
    
    double *a;
} LayerSynBackProp;

void propagateSpikeLayerSynBackProp(LayerSynBackProp  *l, const size_t *ni, const SynSpike *sp, const Constants *c);
void simulateSRMLayerNeuron(LayerSynBackProp *l, const size_t *id_to_sim, const Constants *c);

#endif
