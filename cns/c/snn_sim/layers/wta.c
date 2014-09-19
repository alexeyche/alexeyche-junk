
#include "wta.h"

#include <sim/sim.h>

pthread_spinlock_t sum_prob_spinlock;

LayerWta* createWtaLayer(size_t N, size_t *glob_idx, unsigned char statLevel) {
    LayerWta *l = (LayerWta*)malloc(sizeof(LayerWta));
    l->base = *createPoissonLayer(N, glob_idx, statLevel);
    l->base.need_steps_sync = true;

    l->base.calculateProbability = &calculateProbability_Wta;
    l->base.calculateSpike = &calculateSpike_Wta;
    l->base.calculateDynamics = &calculateDynamics_Wta;
    l->base.deleteLayer = &deleteLayer_Wta;
    l->base.configureLayer = &configureLayer_Wta;
    l->base.toStartValues = &toStartValues_Wta;
    l->base.propagateSpike = &propagateSpike_Wta;
    l->base.allocSynData = &allocSynData_Wta;
    l->base.deallocSynData = &deallocSynData_Wta;
    l->base.printLayer = &printLayer_Wta;
    l->base.serializeLayer = &serializeLayer_Wta;
    l->base.deserializeLayer= &deserializeLayer_Wta;
    l->base.saveStat= &saveStat_Wta;

    l->sum_prob = 0.0;
    l->b = (double*) malloc( sizeof(double) * N );
    pthread_spin_init(&sum_prob_spinlock, 0);
    return(l);
}


void calculateProbability_Wta(LayerPoisson *l, const size_t *ni, const SimContext *s) {
    LayerWta* linh = (LayerWta*) l;
    calculateProbability_Poisson(l, ni, s);

    pthread_spin_lock(&sum_prob_spinlock);
    linh->sum_prob += l->p[*ni];
    pthread_spin_unlock(&sum_prob_spinlock);
}

void calculateSpike_Wta(LayerPoisson *l, const size_t *ni, const SimContext *s) {
    LayerWta* linh = (LayerWta*) l;
    const Constants *c = s->c;
    
    double p_mod = c->wta->__max_freq/linh->sum_prob;
    l->p[*ni] *= p_mod;
    l->M[*ni] *= p_mod;
    calculateSpike_Poisson(l, ni, s);
}

void calculateDynamics_Wta(LayerPoisson *l, const size_t *ni, const SimContext *s) {
    LayerWta* linh = (LayerWta*) l;
    calculateDynamics_Poisson(l, ni, s);

    if(*ni == 0) {
        linh->sum_prob = 0;
    }
}

void propagateSpike_Wta(LayerPoisson *l, const size_t *ni, const SynSpike *sp, const SimContext *s) {
    LayerWta* linh = (LayerWta*) l;
    propagateSpike_Poisson(l, ni, sp, s);

}

// methods common

void toStartValues_Wta(LayerPoisson *l, const Constants *c) {
    LayerWta* linh = (LayerWta*) l;
    toStartValues_Poisson(l, c);
    for(size_t ni=0; ni < l->N; ni++) {
        linh->b[ni] = 0.0;
    }
}

void allocSynData_Wta(LayerPoisson *l) {
    LayerWta* linh = (LayerWta*) l;
    allocSynData_Poisson(l);

}

void deallocSynData_Wta(LayerPoisson *l) {
    LayerWta* linh = (LayerWta*) l;
    deallocSynData_Poisson(l);

}

void printLayer_Wta(LayerPoisson *l) {
    LayerWta* linh = (LayerWta*) l;
    printLayer_Poisson(l);

}

void deleteLayer_Wta(LayerPoisson *l) {
    LayerWta* linh = (LayerWta*) l;
    free(linh->b);
    deleteLayer_Poisson(l);
    
}

void configureLayer_Wta(LayerPoisson *l, const indVector *inputIDs, const indVector *outputIDs, const Constants *c) {
    LayerWta* linh = (LayerWta*) l;
    configureLayer_Poisson(l, inputIDs, outputIDs, c);

}

void serializeLayer_Wta(LayerPoisson *l, FileStream *file, const Sim *s) {
    LayerWta* linh = (LayerWta*) l;
    serializeLayer_Poisson(l, file, s);

//    pMatrixVector *data = TEMPLATE(createVector,pMatrix)();    
//    Matrix *b_m= createMatrix(l->N,1);
//    for(size_t ni=0; ni < l->N; ni++) {
//        setMatrixElement(b_m, ni, 0, linh->b[ni]);
//    }
//    TEMPLATE(insertVector,pMatrix)(data, b_m);
//    saveMatrixList(file, data);
}

#define WTA_LAYER_SERIALIZATION_SIZE 1
void deserializeLayer_Wta(LayerPoisson *l, FileStream *file, const Sim *s) {
    LayerWta* linh = (LayerWta*) l;
    deserializeLayer_Poisson(l, file, s);

//    pMatrixVector *data = readMatrixList(file, WTA_LAYER_SERIALIZATION_SIZE);
//    
//    Matrix *b_m= data->array[0];
//    assert( (b_m->nrow == l->N) && (b_m->ncol == 1) );
//    
//    for(size_t ni=0; ni<l->N; ni++) {
//        linh->b[ni] = getMatrixElement(b_m, ni, 0);
//    }
//
//    TEMPLATE(deleteVector,pMatrix)(data);
}

void saveStat_Wta(LayerPoisson *l, pMatrixVector *mv) { 
    LayerWta* linh = (LayerWta*) l;
    saveStat_Poisson(l, mv);

}
