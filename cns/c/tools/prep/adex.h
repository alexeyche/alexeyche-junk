#ifndef ADEX_LAYER_H
#define ADEX_LAYER_H

#include <math.h>

#include <core/constants.h>

struct AdExLayer {
    size_t N;
    
    size_t *ids;
    unsigned char *fired;    
    double *V;
    double *w;

    bool saveStat;
    doubleVector **stat_V;
    doubleVector **stat_w;
};
typedef struct AdExLayer AdExLayer;

AdExLayer *createAdExLayer(const size_t N, bool saveStat);
void deleteAdExLayer(AdExLayer *l);
void simulateAdExLayerNeuron(AdExLayer *l, const size_t *id_to_sim, const Constants *c);
void toStartValuesAdExLayer(AdExLayer *l, const Constants *c);
void propagateCurrentAdExLayer(AdExLayer *l, const size_t *ni, const double *I);

#endif
