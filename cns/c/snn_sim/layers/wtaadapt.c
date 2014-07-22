
#include "wtaadapt.h"

#include <sim/sim.h>

LayerWtaAdapt* createWtaAdaptLayer(size_t N, size_t *glob_idx, unsigned char statLevel) {
    LayerWtaAdapt *l = (LayerWtaAdapt*)malloc(sizeof(LayerWtaAdapt));
    l->base = *createWtaLayer(N, glob_idx, statLevel);

    LayerPoisson *lbase = (LayerPoisson*)l;
    lbase->calculateProbability = &calculateProbability_WtaAdapt;
    lbase->calculateSpike = &calculateSpike_WtaAdapt;
    lbase->calculateDynamics = &calculateDynamics_WtaAdapt;
    lbase->deleteLayer = &deleteLayer_WtaAdapt;
    lbase->configureLayer = &configureLayer_WtaAdapt;
    lbase->toStartValues = &toStartValues_WtaAdapt;
    lbase->propagateSpike = &propagateSpike_WtaAdapt;
    lbase->allocSynData = &allocSynData_WtaAdapt;
    lbase->deallocSynData = &deallocSynData_WtaAdapt;
    lbase->printLayer = &printLayer_WtaAdapt;
    lbase->serializeLayer = &serializeLayer_WtaAdapt;
    lbase->deserializeLayer= &deserializeLayer_WtaAdapt;
    lbase->saveStat= &saveStat_WtaAdapt;

    l->ga = (double*) malloc(sizeof(double)*N);
    return(l);
}


void calculateProbability_WtaAdapt(LayerPoisson *l, const size_t *ni, const SimContext *s) {
    LayerWtaAdapt* linh = (LayerWtaAdapt*) l;
    calculateProbability_Wta(l, ni, s);
    double adapt_mod = exp(-linh->ga[*ni]);
    l->p[*ni] *= adapt_mod;
    l->M[*ni] *= adapt_mod;
}

void calculateSpike_WtaAdapt(LayerPoisson *l, const size_t *ni, const SimContext *s) {
    LayerWtaAdapt* linh = (LayerWtaAdapt*) l;
    calculateSpike_Wta(l, ni, s);

    const Constants *c = s->c;
    if(l->fired[ *ni ] == 1) {
        linh->ga[*ni] += c->qa;
    }
}

void calculateDynamics_WtaAdapt(LayerPoisson *l, const size_t *ni, const SimContext *s) {
    LayerWtaAdapt* linh = (LayerWtaAdapt*) l;
    calculateDynamics_Wta(l, ni, s);

    const Constants *c = s->c;
    linh->ga[*ni] += - linh->ga[*ni]/c->ta;
}

void propagateSpike_WtaAdapt(LayerPoisson *l, const size_t *ni, const SynSpike *sp, const SimContext *s) {
    LayerWtaAdapt* linh = (LayerWtaAdapt*) l;
    propagateSpike_Wta(l, ni, sp, s);

}

// methods common

void toStartValues_WtaAdapt(LayerPoisson *l, const Constants *c) {
    LayerWtaAdapt* linh = (LayerWtaAdapt*) l;
    toStartValues_Wta(l, c);

}

void allocSynData_WtaAdapt(LayerPoisson *l) {
    LayerWtaAdapt* linh = (LayerWtaAdapt*) l;
    allocSynData_Wta(l);

}

void deallocSynData_WtaAdapt(LayerPoisson *l) {
    LayerWtaAdapt* linh = (LayerWtaAdapt*) l;
    deallocSynData_Wta(l);

}

void printLayer_WtaAdapt(LayerPoisson *l) {
    LayerWtaAdapt* linh = (LayerWtaAdapt*) l;
    printLayer_Wta(l);

}

void deleteLayer_WtaAdapt(LayerPoisson *l) {
    LayerWtaAdapt* linh = (LayerWtaAdapt*) l;
    free(linh->ga);
    deleteLayer_Wta(l);

}

void configureLayer_WtaAdapt(LayerPoisson *l, const indVector *inputIDs, const indVector *outputIDs, const Constants *c) {
    LayerWtaAdapt* linh = (LayerWtaAdapt*) l;
    configureLayer_Wta(l, inputIDs, outputIDs, c);

}

void serializeLayer_WtaAdapt(LayerPoisson *l, FileStream *file, const Constants *c) {
    LayerWtaAdapt* linh = (LayerWtaAdapt*) l;
    serializeLayer_Wta(l, file, c);

    pMatrixVector *data = TEMPLATE(createVector,pMatrix)();    
    Matrix *ga_m= createMatrix(l->N,1);
    for(size_t ni=0; ni < l->N; ni++) {
        setMatrixElement(ga_m, ni, 0, linh->ga[ni]);
    }
    TEMPLATE(insertVector,pMatrix)(data, ga_m);
    saveMatrixList(file, data);
}

#define WTAADAPT_LAYER_SERIALIZATION_SIZE 1
void deserializeLayer_WtaAdapt(LayerPoisson *l, FileStream *file, const Constants *c) {
    LayerWtaAdapt* linh = (LayerWtaAdapt*) l;
    deserializeLayer_Wta(l, file, c);

    pMatrixVector *data = readMatrixList(file, WTAADAPT_LAYER_SERIALIZATION_SIZE);
    
    Matrix *ga_m= data->array[0];
    assert( (ga_m->nrow == l->N) && (ga_m->ncol == 1) );
    
    for(size_t ni=0; ni<l->N; ni++) {
        linh->ga[ni] = getMatrixElement(ga_m, ni, 0);
    }

    TEMPLATE(deleteVector,pMatrix)(data);
}

void saveStat_WtaAdapt(LayerPoisson *l, FileStream *file) {
    LayerWtaAdapt* linh = (LayerWtaAdapt*) l;
    saveStat_Wta(l, file);

}
