
#include <sim.h>

void deleteSim(Sim *s) {
    TEMPLATE(deleteVector,pSRMLayer)(s->layers);
    deleteNetSim(s->ns);
    deleteRuntime(s->rt);
    free(s);
}

SimRuntime* createRuntime() {
    SimRuntime *rt = (SimRuntime*) malloc(sizeof(SimRuntime));
    rt->reset_timeline = TEMPLATE(createVector,double)();
    rt->pattern_classes = TEMPLATE(createVector,ind)();
    rt->timeline_iter = 0;
    rt->Tmax = 0;
    return(rt);
}

void deleteRuntime(SimRuntime *sr) {
    TEMPLATE(deleteVector,double)(sr->reset_timeline);
    free(sr);
}




void configureNetSpikesSim(Sim *s, const char *input_spikes_filename, Constants *c) {
    // filling receiver-oriented connection map
    allocNetSim(s->ns, s->net_size);
    
    configureConnMapNetSim(s->ns, s->layers); 
    if(s->rt) {
        deleteRuntime(s->rt);
        s->rt = createRuntime();
    }

    if(input_spikes_filename) {
        pMatrixVector *ml = readMatrixList(input_spikes_filename);
        assert(ml && ml->size >= 2);
    
        Matrix *inp_m = ml->array[0];
        SpikesList *inp_sl = spikesMatrixToSpikesList(inp_m);
        propagateInputSpikesNetSim(s, inp_sl);

        double Tmax = 0;
        for(size_t ni=0; ni<s->ns->net->size; ni++) {
            if(s->ns->net->list[ni]->size>0) {
                if(s->ns->net->list[ni]->array[ s->ns->net->list[ni]->size-1 ]> Tmax) {
                    Tmax = s->ns->net->list[ni]->array[ s->ns->net->list[ni]->size-1 ];
                }
            }
        }   
        s->rt->Tmax = Tmax + 100;
        deleteSpikesList(inp_sl); 
    
        Matrix *timeline_m = ml->array[1];
        for(size_t ri=0; ri<timeline_m->nrow*timeline_m->ncol; ri++) {
            TEMPLATE(insertVector,double)(s->rt->reset_timeline, timeline_m->vals[ri]);
        }
        Matrix *classes_m = ml->array[2];
        for(size_t ri=0; ri<classes_m->nrow*classes_m->ncol; ri++) {
            TEMPLATE(insertVector,double)(s->rt->pattern_classes, classes_m->vals[ri]);
        }
    
        TEMPLATE(deleteVector,pMatrix)(ml);
    }
}

void configureLayersSim(Sim *s, Constants *c, unsigned char statLevel) {
    s->c = c;
    indVector *inputIDs = TEMPLATE(createVector,ind)();
    for(size_t inp_i=0; inp_i<c->M; inp_i++) {
        TEMPLATE(insertVector,ind)(inputIDs, inp_i);
    }
    
    size_t net_size = s->c->M; 

    size_t neurons_idx = inputIDs->size; // start from last input ID

    for(size_t li=0; li< s->c->layers_size->size; li++) {
        SRMLayer *l = createSRMLayer(c->layers_size->array[li], &neurons_idx, statLevel);
        appendLayerSim(s, l);
        net_size += l->N;
    }   

    s->net_size = net_size;
    indVector *inp = NULL;
    indVector *outp = NULL;
    for(size_t li=0; li< s->c->layers_size->size; li++) {
        SRMLayer *l = s->layers->array[li];
        if(li == 0) {
            inp = inputIDs;
        } else {
            inp = TEMPLATE(copyFromArray,ind)(s->layers->array[li-1]->ids, s->layers->array[li-1]->N);
        }
        if(li+1 < s->c->layers_size->size) {
            outp = TEMPLATE(copyFromArray,ind)(s->layers->array[li+1]->ids, s->layers->array[li+1]->N);
        } else {
            outp = NULL;
        }        
        configureSRMLayer(l, inp, outp, s->c);
        if(inp) {
            TEMPLATE(deleteVector,ind)(inp);
        }
        if(outp) {
            TEMPLATE(deleteVector,ind)(outp);
        }
    }
    TEMPLATE(deleteVector,ind)(inputIDs);
}

size_t getLayerIdOfNeuron(Sim *s, size_t n_id) {
    for(size_t li=0; li<s->layers->size; li++) {
        SRMLayer *l = s->layers->array[li];
        assert(l->N);
        if((l->ids[0] <= n_id) && (l->ids[l->N-1] >= n_id)) {
            return(l->id);
        }
    }
    printf("Can't find layer id for neuron with id %zu\n", n_id);
    exit(1);
}

void configureSynapses(Sim *s, Constants *c) {
    assert(s->ns);
    for(size_t li=0; li < s->layers->size; li++) {
        SRMLayer *l = s->layers->array[li];
        for(size_t ni=0; ni < l->N; ni++) {
            size_t n_id = l->ids[ni];
            for(size_t cons_i=0; cons_i < s->ns->conn_map[n_id]->size; cons_i++) {
                Conn con = s->ns->conn_map[n_id]->array[cons_i];
                
                SRMLayer *l_cons = s->layers->array[con.l_id];
                if(l->nt[ni] == EXC) {
                    setSynapseSpeciality(l_cons, con.n_id, con.syn_id, c->e_exc);
                } else 
                if(l->nt[ni] == INH) {
                    setSynapseSpeciality(l_cons, con.n_id, con.syn_id, c->e_inh);
                }
            }
        }
    }
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
