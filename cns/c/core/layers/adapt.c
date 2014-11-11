
#include "adapt.h"

#include <core/sim/sim.h>

LayerAdapt* createAdaptLayer(size_t N, size_t *glob_idx, unsigned char statLevel) {
    LayerAdapt *l = (LayerAdapt*)malloc(sizeof(LayerAdapt));
    l->base = *createPoissonLayer(N, glob_idx, statLevel);

    l->base.calculateProbability = &calculateProbability_Adapt;
    l->base.calculateSpike = &calculateSpike_Adapt;
    l->base.calculateDynamics = &calculateDynamics_Adapt;
    l->base.deleteLayer = &deleteLayer_Adapt;
    l->base.configureLayer = &configureLayer_Adapt;
    l->base.toStartValues = &toStartValues_Adapt;
    l->base.propagateSpike = &propagateSpike_Adapt;
    l->base.allocSynData = &allocSynData_Adapt;
    l->base.deallocSynData = &deallocSynData_Adapt;
    l->base.printLayer = &printLayer_Adapt;
    l->base.serializeLayer = &serializeLayer_Adapt;
    l->base.deserializeLayer= &deserializeLayer_Adapt;
    l->base.saveStat= &saveStat_Adapt;

    l->ga = (double*) malloc(sizeof(double)*N);
    return(l);
}


void calculateProbability_Adapt(LayerPoisson *l, const size_t *ni, const SimContext *s) {
    LayerAdapt* linh = (LayerAdapt*) l;
    calculateProbability_Poisson(l, ni, s);
    double adapt_mod = exp(-linh->ga[*ni]);
    l->p[*ni] *= adapt_mod;
    l->M[*ni] *= adapt_mod;
}

void calculateSpike_Adapt(LayerPoisson *l, const size_t *ni, const SimContext *s) {
    LayerAdapt* linh = (LayerAdapt*) l;
    calculateSpike_Poisson(l, ni, s);

    const Constants *c = s->c;
    if(l->fired[ *ni ] == 1) {
        linh->ga[*ni] += c->qa;
    }
}

void calculateDynamics_Adapt(LayerPoisson *l, const size_t *ni, const SimContext *s) {
    LayerAdapt* linh = (LayerAdapt*) l;
    calculateDynamics_Poisson(l, ni, s);

    const Constants *c = s->c;
    linh->ga[*ni] += - linh->ga[*ni]/c->ta;
}

void propagateSpike_Adapt(LayerPoisson *l, const size_t *ni, const SynSpike *sp, const SimContext *s) {
    LayerAdapt* linh = (LayerAdapt*) l;
    propagateSpike_Poisson(l, ni, sp, s);

}

// methods common

void toStartValues_Adapt(LayerPoisson *l, const Constants *c) {
    LayerAdapt* linh = (LayerAdapt*) l;
    toStartValues_Poisson(l, c);

}

void allocSynData_Adapt(LayerPoisson *l) {
    LayerAdapt* linh = (LayerAdapt*) l;
    allocSynData_Poisson(l);

}

void deallocSynData_Adapt(LayerPoisson *l) {
    LayerAdapt* linh = (LayerAdapt*) l;
    deallocSynData_Poisson(l);

}

void printLayer_Adapt(LayerPoisson *l) {
    LayerAdapt* linh = (LayerAdapt*) l;
    printLayer_Poisson(l);

}

void deleteLayer_Adapt(LayerPoisson *l) {
    LayerAdapt* linh = (LayerAdapt*) l;
    free(linh->ga);
    deleteLayer_Poisson(l);
}

void configureLayer_Adapt(LayerPoisson *l, const indVector *inputIDs, const indVector *outputIDs, const Constants *c) {
    LayerAdapt* linh = (LayerAdapt*) l;
    configureLayer_Poisson(l, inputIDs, outputIDs, c);

}

void serializeLayer_Adapt(LayerPoisson *l, FileStream *file, const Sim *s) {
    LayerAdapt* linh = (LayerAdapt*) l;
    serializeLayer_Poisson(l, file, s);

    pMatrixVector *data = TEMPLATE(createVector,pMatrix)();    
    Matrix *ga_m= createMatrix(l->N,1);
    for(size_t ni=0; ni < l->N; ni++) {
        setMatrixElement(ga_m, ni, 0, linh->ga[ni]);
    }
    TEMPLATE(insertVector,pMatrix)(data, ga_m);
    saveMatrixList(file, data);
}

#define ADAPT_LAYER_SERIALIZATION_SIZE 1
void deserializeLayer_Adapt(LayerPoisson *l, FileStream *file, const Sim *s) {
    LayerAdapt* linh = (LayerAdapt*) l;
    deserializeLayer_Poisson(l, file, s);

    pMatrixVector *data = readMatrixList(file, ADAPT_LAYER_SERIALIZATION_SIZE);
    
    Matrix *ga_m= data->array[0];
    assert( (ga_m->nrow == l->N) && (ga_m->ncol == 1) );
    
    for(size_t ni=0; ni<l->N; ni++) {
        linh->ga[ni] = getMatrixElement(ga_m, ni, 0);
    }

    TEMPLATE(deleteVector,pMatrix)(data);
}

void saveStat_Adapt(LayerPoisson *l, pMatrixVector *mv) { 
    LayerAdapt* linh = (LayerAdapt*) l;
    saveStat_Poisson(l, mv);
}
