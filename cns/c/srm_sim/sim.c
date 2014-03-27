
#include "sim.h"

Sim* createSim() {
    Sim *s = (Sim*) malloc(sizeof(Sim));
    s->ns = createNetSim();
    s->layers = TEMPLATE(createVector,pSRMLayer)(); 
    s->rt = createRuntime();
    return(s);
}


void appendLayerSim(Sim *s, SRMLayer *l) {
    TEMPLATE(insertVector,pSRMLayer)(s->layers, l);
}


void deleteSim(Sim *s) {
    TEMPLATE(deleteVector,pSRMLayer)(s->layers);
    deleteNetSim(s->ns);
    deleteRuntime(s->rt);
    free(s);
}

SimRuntime* createRuntime() {
    SimRuntime *rt = (SimRuntime*) malloc(sizeof(SimRuntime));
    rt->t = 0;
    rt->input_spikes_iter = TEMPLATE(createVector,ind)();
    return(rt);
}

void deleteRuntime(SimRuntime *sr) {
    TEMPLATE(deleteVector,ind)(rt->input_spikes_iter);
    free(rt);
}

void allocRuntime(SimRuntime *rt, size_t net_size) {
    for(size_t inp_i=0; inp_i < net_size; inp_i++) {
        TEMPLATE(insertVector,ind)(rt->input_spikes_iter, 0);
    }
}

void configureSim(Sim *s, Constants *c) {
    indVector *inputIDs = TEMPLATE(createVector,ind)();
    for(size_t inp_i=0; inp_i<c->M; inp_i++) {
        TEMPLATE(insertVector,ind)(inputIDs, inp_i);
    }
    
    size_t net_size = c->M; 

    size_t neurons_idx = inputIDs->size; // start from last input ID
    for(size_t li=0; li< c->layers_size->size; li++) {
        SRMLayer *l = createSRMLayer(c->layers_size->array[li], &neurons_idx);
        if(li == 0) {
            configureSRMLayer(l, inputIDs, c);
        } else {
            configureSRMLayer(l, NULL, c);
        }
        appendLayerSim(s, l);
        net_size += l->N;
    }   
    // filling receiver-oriented connection map
    printSRMLayer(s->layers->array[0]);
    allocNetSim(s->ns, net_size);
    
    configureConnMapNetSim(s->ns, s->layers); 

    pMatrixVector *ml = readMatrixList(c->input_spikes_filename);
    assert(ml && ml->size == 1);
    Matrix *inp_m = ml->array[0];
    SpikesList *inp_sl = spikesMatrixToSpikesList(inp_m);
    
    propagateInputSpikesNetSim(s->ns, inp_sl);

    if(rt) {
        deleteRuntime(s->rt);
        s->rt = createRuntime();
    }
    allocRuntime(s->rt, s->ns->size);
    
    deleteSpikesList(inp_sl); 
    TEMPLATE(deleteVector,pMatrix)(ml);
    TEMPLATE(deleteVector,ind)(inputIDs);
}



void simulate(Sim *s, Constants *c) {
    for(size_t li=0; s->layers->size; li++) {
        SRMLayer *l = s->layers->array[li];
        for(size_t ni=0; ni < l->N; ni++) {

            simulateSRMLayerNeuron(l, &ni, c);
        }
    }
}

