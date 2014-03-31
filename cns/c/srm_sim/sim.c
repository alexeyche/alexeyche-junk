
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

void configreNetSpikesSim(Sim *s, Constants *c) {
    // filling receiver-oriented connection map
    allocNetSim(s->ns, s->net_size);
    
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
    s->rt->Tmax = Tmax + 100;
    deleteSpikesList(inp_sl); 
    TEMPLATE(deleteVector,pMatrix)(ml);
}

void configureLayersSim(Sim *s, Constants *c, bool saveStat) {
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
    s->net_size = net_size;
    TEMPLATE(deleteVector,ind)(inputIDs);
}

const SynSpike* getInputSpike(NetSim *ns, SimRuntime *sr, const size_t *n_id, const Constants *c) {
    if(*n_id > ns->size) printf("error in doWeHaveSpike\n");
    if(sr->input_spikes_iter->array[ *n_id ] < ns->input_spikes_queue[ *n_id ]->size) {
        while(( ns->input_spikes_queue[ *n_id ]->array[ sr->input_spikes_iter->array[ *n_id ] ].t < sr->t)&&
              (sr->input_spikes_iter->array[ *n_id ] < ns->input_spikes_queue[ *n_id ]->size)) {
//            printf("missing spike on neuron %zu at syn %zu at %f\n", *n_id, ns->input_spikes_queue[ *n_id ]->array[ sr->input_spikes_iter->array[ *n_id ] ].syn_id, ns->input_spikes_queue[ *n_id ]->array[ sr->input_spikes_iter->array[ *n_id ] ].t);
            sr->input_spikes_iter->array[ *n_id ] += 1;
        }
        if(ns->input_spikes_queue[ *n_id ]->array[ sr->input_spikes_iter->array[ *n_id ] ].t < sr->t+c->dt) {
//           printf("input_iter: %zu\n", sr->input_spikes_iter->array[ *n_id ]);
//            printf("propagate spike on neuron %zu at syn %zu at %f\n", *n_id, ns->input_spikes_queue[ *n_id ]->array[ sr->input_spikes_iter->array[ *n_id ] ].syn_id, ns->input_spikes_queue[ *n_id ]->array[ sr->input_spikes_iter->array[ *n_id ] ].t);
           const SynSpike *to_return = &ns->input_spikes_queue[ *n_id ]->array[ sr->input_spikes_iter->array[ *n_id ] ];
           sr->input_spikes_iter->array[ *n_id ] += 1;
//           printf("%f: returning %zu %zu %f\n", sr->t, to_return->n_id, to_return->syn_id, to_return->t);
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
//                    printf("inserting int act syns: %zu %zu %f\n", sp->n_id, sp->syn_id, sp->t);
                    TEMPLATE(addValueLList,ind)(l->active_syn_ids[ni], sp->syn_id);
                } 
//                else printf("not inserting int act syns: %zu %zu %f because %f > %f\n", sp->n_id, sp->syn_id, sp->t, l->syn[ni][ sp->syn_id ] , SYN_ACT_TOL);

                
//                printf("setting synapse %zu act to e0\n", sp->syn_id);
                l->syn[ni][ sp->syn_id ] += l->syn_spec[ni][ sp->syn_id ] * c->e0;
                l->syn[ ni ][ sp->syn_id ] *= l->a[ ni ];
                l->syn_fired[ni][ sp->syn_id ] = 1;
            }
            simulateSRMLayerNeuron(l, &ni, c);
            if(l->fired[ni] == 1) {
                propagateSpike(s->ns, l->ids[ni], s->rt->t);
                //printf("We have a spike in %zu at %f\n", l->ids[ni], s->rt->t);
                l->fired[ni] = 0;
            }
        }
    }
    s->rt->t += c->dt;
//    if(s->rt->t > 1000) exit(1);
}

#define MATRIX_PER_LAYER 4
void loadLayersFromFile(Sim *s, const char *model_fname, Constants *c, bool saveStat) {
    pMatrixVector* data = readMatrixList(model_fname);
    assert(data != NULL);
    size_t net_size = c->M;
    for(size_t li=0; li< c->layers_size->size; li++) {
        pMatrixVector* data_layer = TEMPLATE(createVector,pMatrix)();
        for(size_t di=0; di < MATRIX_PER_LAYER; di++) {
            TEMPLATE(insertVector,pMatrix)(data_layer, data->array[di + MATRIX_PER_LAYER*li]);
        }
        size_t neurons_idx = c->M;
        SRMLayer *l = createSRMLayer(c->layers_size->array[li], &neurons_idx, saveStat);
        loadSRMLayer(l, c, data_layer);
        appendLayerSim(s, l);
        TEMPLATE(deleteVectorNoDestroy,pMatrix)(data_layer);
        net_size += l->N;
    }
    s->net_size = net_size;
    TEMPLATE(deleteVector,pMatrix)(data);
}

void saveLayersToFile(Sim *s, const char *model_fname) {
    pMatrixVector *data = TEMPLATE(createVector,pMatrix)();
    for(size_t li=0; li<s->layers->size; li++) {
        pMatrixVector *data_layer = serializeSRMLayer(s->layers->array[li]);
        assert(data_layer->size == MATRIX_PER_LAYER);
        for(size_t di=0; di < data_layer->size; di++) {
            TEMPLATE(insertVector,pMatrix)(data, data_layer->array[di]);
        }
        TEMPLATE(deleteVectorNoDestroy,pMatrix)(data_layer);
    }
    assert(data->size > 0);
    assert(model_fname);
    saveMatrixList(model_fname, data);
    TEMPLATE(deleteVector,pMatrix)(data);
}

