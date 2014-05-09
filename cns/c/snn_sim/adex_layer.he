#ifndef ADEX_LAYER_H
#define ADEX_LAYER_H

struct AdExLayer {
    size_t N;
    
    size_t *ids;
    unsigned char *fired;    
    double *V;
    double *w;
};

AdExLayer *createAdExLayer(const size_t N);
void deleteAdExLayer(AdExLayer *l);
void simulateAdExLayerNeuron(AdExLayer *l, const size_t *id_to_sim, const Constants *c);
void toStartValuesAdExLayer(AdExLayer *l);

#endif
