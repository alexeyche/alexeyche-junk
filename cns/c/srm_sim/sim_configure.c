
#include "sim.h"

void deleteSim(Sim *s) {
    TEMPLATE(deleteVector,pSRMLayer)(s->layers);
    deleteNetSim(s->ns);
    deleteRuntime(s->rt);
    free(s);
}

SimRuntime* createRuntime() {
    SimRuntime *rt = (SimRuntime*) malloc(sizeof(SimRuntime));
    rt->input_spikes_iter = TEMPLATE(createVector,ind)();
    rt->spikes_iter = TEMPLATE(createVector,ind)();
    rt->reset_timeline = TEMPLATE(createVector,double)();
    rt->timeline_iter = 0;
    return(rt);
}

void deleteRuntime(SimRuntime *sr) {
    TEMPLATE(deleteVector,ind)(sr->input_spikes_iter);
    TEMPLATE(deleteVector,ind)(sr->spikes_iter);
    TEMPLATE(deleteVector,double)(sr->reset_timeline);
    free(sr);
}


void allocRuntime(SimRuntime *rt, size_t net_size) {
    for(size_t inp_i=0; inp_i < net_size; inp_i++) {
        TEMPLATE(insertVector,ind)(rt->input_spikes_iter, 0);
        TEMPLATE(insertVector,ind)(rt->spikes_iter, 0);
    }
}

void configreNetSpikesSim(Sim *s, Constants *c) {
    // filling receiver-oriented connection map
    allocNetSim(s->ns, s->net_size);
    
    configureConnMapNetSim(s->ns, s->layers); 
    if(s->rt) {
        deleteRuntime(s->rt);
        s->rt = createRuntime();
    }
    allocRuntime(s->rt, s->ns->size);

    pMatrixVector *ml = readMatrixList(c->input_spikes_filename);
    assert(ml && ml->size >= 2);
    
    Matrix *inp_m = ml->array[0];
    SpikesList *inp_sl = spikesMatrixToSpikesList(inp_m);
    propagateInputSpikesNetSim(s, inp_sl);

    double Tmax = 0;
    for(size_t ni=0; ni<s->ns->net->size; ni++) {
        if(s->ns->net->list[ni]->array[ s->ns->net->list[ni]->size-1 ]> Tmax) {
            Tmax = s->ns->net->list[ni]->array[ s->ns->net->list[ni]->size-1 ];
        }
    }   
    s->rt->Tmax = Tmax + 100;
    deleteSpikesList(inp_sl); 
    
    Matrix *timeline_m = ml->array[1];
    for(size_t ri=0; ri<timeline_m->nrow*timeline_m->ncol; ri++) {
        TEMPLATE(insertVector,double)(s->rt->reset_timeline, timeline_m->vals[ri]);
    }
    
    TEMPLATE(deleteVector,pMatrix)(ml);
}

void configureLayersSim(Sim *s, Constants *c, bool saveStat) {
    s->c = c;
    indVector *inputIDs = TEMPLATE(createVector,ind)();
    for(size_t inp_i=0; inp_i<c->M; inp_i++) {
        TEMPLATE(insertVector,ind)(inputIDs, inp_i);
    }
    
    size_t net_size = s->c->M; 

    size_t neurons_idx = inputIDs->size; // start from last input ID
    for(size_t li=0; li< s->c->layers_size->size; li++) {
        SRMLayer *l = createSRMLayer(c->layers_size->array[li], &neurons_idx, saveStat);
        if(li == 0) {
            configureSRMLayer(l, inputIDs, s->c);
        } else {
            configureSRMLayer(l, NULL, s->c);
        }
        appendLayerSim(s, l);
        net_size += l->N;
    }   
    s->net_size = net_size;
    TEMPLATE(deleteVector,ind)(inputIDs);
}

void configureSimAttr(Sim *s) {
    size_t num_neurons = 0;
    for(size_t li=0; li<s->layers->size; li++) {
        num_neurons += s->layers->array[li]->N;
    }
    s->na = (NeuronAddress*) malloc(num_neurons * sizeof(NeuronAddress));
    s->num_neurons = num_neurons;
    size_t iter = 0;
    for(size_t li=0; li<s->layers->size; li++) {
        for(size_t ni=0; ni<s->layers->array[li]->N; ni++) {
            s->na[iter].n_id = ni;
            s->na[iter].layer_id = li;
            ++iter;
        }
    }
}
