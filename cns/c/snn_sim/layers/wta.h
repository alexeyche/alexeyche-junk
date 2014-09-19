#ifndef LAYER_WTA_H
#define LAYER_WTA_H

#include <pthread.h>


#include <layers/poisson.h>

struct Sim;

typedef struct {
    LayerPoisson base;

    double sum_prob;
    double *b;
} LayerWta;


LayerWta* createWtaLayer(size_t N, size_t *glob_idx, unsigned char statLevel);

// methods sim
void calculateProbability_Wta(LayerPoisson *l, const size_t *ni, const struct SimContext *s);
void calculateSpike_Wta(LayerPoisson *l, const size_t *ni, const struct SimContext *s);
void calculateDynamics_Wta(LayerPoisson *l, const size_t *ni, const struct SimContext *s);
void propagateSpike_Wta(LayerPoisson *l, const size_t *ni, const SynSpike *sp, const struct SimContext *s);

// methods common
void toStartValues_Wta(LayerPoisson *l, const Constants *c);
void allocSynData_Wta(LayerPoisson *l);
void deallocSynData_Wta(LayerPoisson *l);
void printLayer_Wta(LayerPoisson *l);
void deleteLayer_Wta(LayerPoisson *l);
void configureLayer_Wta(LayerPoisson *l, const indVector *inputIDs, const indVector *outputIDs, const Constants *c);
void serializeLayer_Wta(LayerPoisson *l, FileStream *file, const struct Sim *s);
void deserializeLayer_Wta(LayerPoisson *l, FileStream *file, const struct Sim *s);
void saveStat_Wta(LayerPoisson *l, pMatrixVector *mv); 

#endif    
