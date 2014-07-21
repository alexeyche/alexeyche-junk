#ifndef LAYER_ADAPT_H
#define LAYER_ADAPT_H

#include <layers/poisson.h>


typedef struct {
    LayerPoisson base;

    double *ga;
} LayerAdapt;

LayerAdapt* createAdaptLayer(size_t N, size_t *glob_idx, unsigned char statLevel);

// methods sim
void calculateProbability_Adapt(LayerPoisson *l, const size_t *ni, const struct SimContext *s);
void calculateSpike_Adapt(LayerPoisson *l, const size_t *ni, const struct SimContext *s);
void calculateDynamics_Adapt(LayerPoisson *l, const size_t *ni, const struct SimContext *s);
void propagateSpike_Adapt(LayerPoisson *l, const size_t *ni, const SynSpike *sp, const struct SimContext *s);

// methods common
void toStartValues_Adapt(LayerPoisson *l, const Constants *c);
void allocSynData_Adapt(LayerPoisson *l);
void deallocSynData_Adapt(LayerPoisson *l);
void printLayer_Adapt(LayerPoisson *l);
void deleteLayer_Adapt(LayerPoisson *l);
void configureLayer_Adapt(LayerPoisson *l, const indVector *inputIDs, const indVector *outputIDs, const Constants *c);
void serializeLayer_Adapt(LayerPoisson *l, FileStream *file, const Constants *c);
void deserializeLayer_Adapt(LayerPoisson *l, FileStream *file, const Constants *c);
void saveStat_Adapt(LayerPoisson *l, FileStream *file);

#endif    
