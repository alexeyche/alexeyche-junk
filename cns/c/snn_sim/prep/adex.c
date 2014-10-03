
#include "adex.h"

AdExLayer *createAdExLayer(const size_t N, bool saveStat) {
    AdExLayer *l = (AdExLayer*) malloc( sizeof(AdExLayer) );
    l->N = N;
    l->ids = (size_t*) malloc( l->N * sizeof(size_t) );
    l->fired = (unsigned char*) malloc( l->N * sizeof(unsigned char) );
    l->V = (double*) malloc( l->N * sizeof(double) );
    l->w = (double*) malloc( l->N * sizeof(double) );
    l->saveStat = saveStat;
    if(l->saveStat) {
        l->stat_V = (doubleVector**) malloc( l->N * sizeof(doubleVector*) );
        l->stat_w = (doubleVector**) malloc( l->N * sizeof(doubleVector*) );
        for(size_t ni=0; ni<l->N; ni++) {
            l->stat_V[ni] = TEMPLATE(createVector,double)();
            l->stat_w[ni] = TEMPLATE(createVector,double)();
        }
    }
    return(l);
}


void deleteAdExLayer(AdExLayer *l) {
    free(l->V);
    free(l->w);
    free(l->ids);
    free(l->fired);
    if(l->saveStat) {
        for(size_t ni=0; ni<l->N; ni++) {
            TEMPLATE(deleteVector,double)(l->stat_V[ni]);
            TEMPLATE(deleteVector,double)(l->stat_w[ni]);
        }
    }
    l->N = 0;
    free(l);
}

void toStartValuesAdExLayer(AdExLayer *l, const Constants *c) {
    for(size_t ni=0; ni<l->N; ni++) {
        l->V[ni] = c->adex->EL;
        l->w[ni] = 0;
    }
}

void propagateCurrentAdExLayer(AdExLayer *l, const size_t *ni, const double *I) {
    l->V[ *ni ] += *I;
}

void simulateAdExLayerNeuron(AdExLayer *l, const size_t *id_to_sim, const Constants *c) {
    double dV = - c->adex->gL * ( l->V[ *id_to_sim ] - c->adex->EL );
    if(c->adex->slope > 0.0) {
        dV += c->adex->gL * c->adex->slope * exp( (l->V[ *id_to_sim ] - c->adex->Vtr)/c->adex->slope )   
    }                  
    dV += -l->w[ *id_to_sim ];
    double dw = c->adex->a * ( l->V[ *id_to_sim ] - c->adex->EL ) - l->w[ *id_to_sim ]; 

    l->V[ *id_to_sim ] += c->dt * ( dV/c->adex->C );
    l->w[ *id_to_sim ] += c->dt * ( dw/c->adex->tau_w );
    if( l->V[ *id_to_sim ] >= c->adex->Vtr ) {
        l->V[ *id_to_sim ] = c->adex->EL;
        l->w[ *id_to_sim ] += c->adex->b;
        l->fired[ *id_to_sim ] = 1;
    }
    if(l->saveStat) {
        TEMPLATE(insertVector,double)(l->stat_V[ *id_to_sim ], l->V[ *id_to_sim ]);
        TEMPLATE(insertVector,double)(l->stat_w[ *id_to_sim ], l->w[ *id_to_sim ]);
    }
}
