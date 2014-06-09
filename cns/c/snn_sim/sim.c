
#include <sim.h>

Sim* createSim(size_t nthreads, unsigned char stat_level, Constants *c) {
    Sim *s = (Sim*) malloc(sizeof(Sim));
    s->ns = createNetSim();
    s->layers = TEMPLATE(createVector,pSRMLayer)(); 
    s->rt = createRuntime();
    s->nthreads = nthreads;
    s->global_reward = 0;
    s->stat_level = stat_level;
    if(s->stat_level > 0) {
        s->stat_global_reward = TEMPLATE(createVector,double)();
    }
    return(s);
}


void appendLayerSim(Sim *s, SRMLayer *l) {
    TEMPLATE(insertVector,pSRMLayer)(s->layers, l);
    l->id = s->layers->size - 1;
}

void deleteSim(Sim *s) {
    TEMPLATE(deleteVector,pSRMLayer)(s->layers);
    deleteNetSim(s->ns);
    deleteRuntime(s->rt);
    if(s->stat_level > 1) {
        TEMPLATE(deleteVector,double)(s->stat_global_reward);
    }

    free(s);
}
