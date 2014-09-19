#ifndef LAYER_WTAADAPT_H
#define LAYER_WTAADAPT_H

#include <layers/wta.h>

struct Sim;

typedef struct {
    LayerWta base;

    double *ga;
} LayerWtaAdapt;

LayerWtaAdapt* createWtaAdaptLayer(size_t N, size_t *glob_idx, unsigned char statLevel);

// methods sim
void calculateProbability_WtaAdapt(LayerPoisson *l, const size_t *ni, const struct SimContext *s);
void calculateSpike_WtaAdapt(LayerPoisson *l, const size_t *ni, const struct SimContext *s);
void calculateDynamics_WtaAdapt(LayerPoisson *l, const size_t *ni, const struct SimContext *s);
void propagateSpike_WtaAdapt(LayerPoisson *l, const size_t *ni, const SynSpike *sp, const struct SimContext *s);

// methods common
void toStartValues_WtaAdapt(LayerPoisson *l, const Constants *c);
void allocSynData_WtaAdapt(LayerPoisson *l);
void deallocSynData_WtaAdapt(LayerPoisson *l);
void printLayer_WtaAdapt(LayerPoisson *l);
void deleteLayer_WtaAdapt(LayerPoisson *l);
void configureLayer_WtaAdapt(LayerPoisson *l, const indVector *inputIDs, const indVector *outputIDs, const Constants *c);
void serializeLayer_WtaAdapt(LayerPoisson *l, FileStream *file, const struct Sim *s);
void deserializeLayer_WtaAdapt(LayerPoisson *l, FileStream *file, const struct Sim *s);
void saveStat_WtaAdapt(LayerPoisson *l, pMatrixVector *mv); 

#endif    
