


AdExLayer *createAdExLayer(const size_t N) {
    AdExLayer *l = (AdExLayer*) malloc( sizeof(AdExLayer) );
    l->N = N;
    l->ids = (size_t*) malloc( l->N * sizeof(size_t) );
    l->fired = (unsigned char*) malloc( l->N * sizeof(unsigned char) );
    l->V = (double*) malloc( l->N * sizeof(double) );
    l->w = (double*) malloc( l->N * sizeof(double) );
}


void deleteAdExLayer(AdExLayer *l) {
    free(l->V);
    free(l->w);
    free(l->ids);
    free(l->fired);
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
    double dV = - c->adex->gL * ( l->V[ *id_to_sim ] - c->adex->EL ) +  \
                  c->adex->gL * c->adex->slope * exp( (l->V[ *id_to_sim ] - c->adex->Vtr)/c->adex->slope ) - l->w[ *id_to_sim ];   
    double dw = c->adex->a * ( l->V[ *id_to_sim ] - c->adex->EL ) - l->w[ *id_to_sim ]; 

    l->V[ *id_to_sim ] += c->dt * ( dV/c->adex->C );
    l->w[ *id_to_sim ] += c->dt * ( dw/c->adex->tau_w );
    if( l->V[ *id_to_sim ] >= 0.0 ) {
        l->V[ *id_to_sim ] = c->adex->EL;
        l->w[ *id_to_sim ] += c->adex->b;
        l->fired[ *id_to_sim ] = 1;
    }
}
