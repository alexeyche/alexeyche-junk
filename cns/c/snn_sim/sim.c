
#include <sim.h>

Sim* createSim(size_t nthreads) {
    Sim *s = (Sim*) malloc(sizeof(Sim));
    s->ns = createNetSim();
    s->layers = TEMPLATE(createVector,pSRMLayer)(); 
    s->rt = createRuntime();
    s->nthreads = nthreads;
    return(s);
}


void appendLayerSim(Sim *s, SRMLayer *l) {
    TEMPLATE(insertVector,pSRMLayer)(s->layers, l);
    l->id = s->layers->size - 1;
}


