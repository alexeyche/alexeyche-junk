
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
    TEMPLATE(deleteVector,ind)(sr->input_spikes_iter);
    free(sr);
}


void allocRuntime(SimRuntime *rt, size_t net_size) {
    for(size_t inp_i=0; inp_i < net_size; inp_i++) {
        TEMPLATE(insertVector,ind)(rt->input_spikes_iter, 0);
    }
}



void configureSim(Sim *s, Constants *c, bool saveStat) {
    indVector *inputIDs = TEMPLATE(createVector,ind)();
    for(size_t inp_i=0; inp_i<c->M; inp_i++) {
        TEMPLATE(insertVector,ind)(inputIDs, inp_i);
    }
    
    size_t net_size = c->M; 

    size_t neurons_idx = inputIDs->size; // start from last input ID
    for(size_t li=0; li< c->layers_size->size; li++) {
        SRMLayer *l = createSRMLayer(c->layers_size->array[li], &neurons_idx, saveStat);
        if(li == 0) {
            configureSRMLayer(l, inputIDs, c);
        } else {
            configureSRMLayer(l, NULL, c);
        }
        appendLayerSim(s, l);
        net_size += l->N;
    }   
    // filling receiver-oriented connection map
    allocNetSim(s->ns, net_size);
    
    configureConnMapNetSim(s->ns, s->layers); 

    pMatrixVector *ml = readMatrixList(c->input_spikes_filename);
    assert(ml && ml->size == 1);
    Matrix *inp_m = ml->array[0];
    SpikesList *inp_sl = spikesMatrixToSpikesList(inp_m);
    propagateInputSpikesNetSim(s->ns, inp_sl);

    if(s->rt) {
        deleteRuntime(s->rt);
        s->rt = createRuntime();
    }
    allocRuntime(s->rt, s->ns->size);
    double Tmax = 0;
    for(size_t ni=0; ni<s->ns->net->size; ni++) {
        if(s->ns->net->list[ni]->array[ s->ns->net->list[ni]->size-1 ]> Tmax) {
            Tmax = s->ns->net->list[ni]->array[ s->ns->net->list[ni]->size-1 ];
        }
    }   
    s->rt->Tmax = Tmax;
    deleteSpikesList(inp_sl); 
    TEMPLATE(deleteVector,pMatrix)(ml);
    TEMPLATE(deleteVector,ind)(inputIDs);
}

const SynSpike* getInputSpike(NetSim *ns, SimRuntime *sr, const size_t *n_id, const Constants *c) {
    if(*n_id > ns->size) printf("error in doWeHaveSpike\n");
    if(sr->input_spikes_iter->array[ *n_id ] < ns->input_spikes_queue[ *n_id ]->size) {
        while(( ns->input_spikes_queue[ *n_id ]->array[ sr->input_spikes_iter->array[ *n_id ] ].t < sr->t)&&
              (sr->input_spikes_iter->array[ *n_id ] < ns->input_spikes_queue[ *n_id ]->size)) {
            sr->input_spikes_iter->array[ *n_id ] += 1;
        }
        if(ns->input_spikes_queue[ *n_id ]->array[ sr->input_spikes_iter->array[ *n_id ] ].t < sr->t+c->dt) {
//           printf("input_iter: %zu\n", sr->input_spikes_iter->array[ *n_id ]);
           const SynSpike *to_return = &ns->input_spikes_queue[ *n_id ]->array[ sr->input_spikes_iter->array[ *n_id ] ];
           sr->input_spikes_iter->array[ *n_id ] += 1;
  //         printf("returning %zu %zu %f\n", to_return->n_id, to_return->syn_id, to_return->t);
           return(to_return);
        }
    }
    return(NULL);
}

void simulate(Sim *s, const Constants *c) {
    const SynSpike *sp;
    for(size_t li=0; li<s->layers->size; li++) {
        SRMLayer *l = s->layers->array[li];
        for(size_t ni=0; ni < l->N; ni++) {
//            printf("%f simulating %zu\n", s->rt->t, l->ids[ni]);
            while( (sp = getInputSpike(s->ns, s->rt, &l->ids[ni], c)) != NULL) {
                if(l->syn[ni][ sp->syn_id ] < SYN_ACT_TOL) {
                    //printf("inserting int act syns: %zu %zu %f\n", sp->n_id, sp->syn_id, sp->t);
                    TEMPLATE(insertVector,ind)(l->active_syn_ids[ni], sp->syn_id);
                }
//                printf("setting synapse %zu act to e0\n", sp->syn_id);
                l->syn[ni][ sp->syn_id ] += l->syn_spec[ni][ sp->syn_id ] * c->e0;
                l->syn_fired[ni][ sp->syn_id ] = 1;
            }
            simulateSRMLayerNeuron(l, &ni, c);
            if(l->fired[ni] == 1) {
                printf("We have a spike in %zu at %f\n", l->ids[ni], s->rt->t);
                l->fired[ni] = 0;
            }
        }
    }
    s->rt->t += c->dt;
}

