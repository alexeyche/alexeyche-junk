
#include <sim/sim.h>


void configureNetSpikesSim(Sim *s, Constants *c) {
    // filling receiver-oriented connection map
    assert(s->impl->net_size>0);
    allocNetSim(s->ns, s->impl->net_size);
    
    configureConnMapNetSim(s->ns, s->layers); 
}



void configureLayersSim(Sim *s, Constants *c) {
    s->ctx->c = c;

    indVector *inputIDs = TEMPLATE(createVector,ind)();
    for(size_t inp_i=0; inp_i<c->M; inp_i++) {
        TEMPLATE(insertVector,ind)(inputIDs, inp_i);
    }
    
    size_t net_size = c->M; 
    size_t neurons_idx = inputIDs->size; // start from last input ID

    for(size_t li=0; li< c->lc->size; li++) {
        LayerConstants *lc = getLayerConstantsC(c, li);
        LayerPoisson *l;
        if(lc->neuron_type == EPoissonLayer) {
            l = createPoissonLayer(lc->N, &neurons_idx, s->ctx->stat_level);
        } else
        if(lc->neuron_type == EWtaLayer) {
            l = (LayerPoisson*)createWtaLayer(lc->N, &neurons_idx, s->ctx->stat_level);
        } else 
        if(lc->neuron_type == EAdaptLayer) {
            l = (LayerPoisson*)createAdaptLayer(lc->N, &neurons_idx, s->ctx->stat_level);
        } else 
        if(lc->neuron_type == EWtaAdaptLayer) {
            l = (LayerPoisson*)createWtaAdaptLayer(lc->N, &neurons_idx, s->ctx->stat_level);
        } else 
        {
            printf("Unknown layer type\n");
            exit(1);
        }
        appendLayerSim(s, l);
        net_size += l->N;
    }   

    s->impl->net_size = net_size;
    indVector *inp = NULL;
    indVector *outp = NULL;
    for(size_t li=0; li< c->lc->size; li++) {
        LayerPoisson *l = s->layers->array[li];
        if(li == 0) {
            inp = inputIDs;
        } else {
            inp = TEMPLATE(copyFromArray,ind)(s->layers->array[li-1]->ids, s->layers->array[li-1]->N);
        }
        if(li+1 < c->lc->size) {
            outp = TEMPLATE(copyFromArray,ind)(s->layers->array[li+1]->ids, s->layers->array[li+1]->N);
        } else {
            outp = NULL;
        }        
        l->configureLayer(l, inp, outp, c);
        if(inp) {
            TEMPLATE(deleteVector,ind)(inp);
        }
        if(outp) {
            TEMPLATE(deleteVector,ind)(outp);
        }
    }
    TEMPLATE(deleteVector,ind)(inputIDs);
}



void configureSynapses(Sim *s, Constants *c) {
    assert(s->ns);
    for(size_t li=0; li < s->layers->size; li++) {
        LayerPoisson *l = s->layers->array[li];
        for(size_t ni=0; ni < l->N; ni++) {
            size_t n_id = l->ids[ni];
            for(size_t cons_i=0; cons_i < s->ns->conn_map[n_id]->size; cons_i++) {
                Conn con = s->ns->conn_map[n_id]->array[cons_i];
                
                LayerPoisson *l_cons = s->layers->array[con.l_id];
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

void configureSimImpl(Sim *s) {
    size_t num_neurons = 0;
    for(size_t li=0; li<s->layers->size; li++) {
        num_neurons += s->layers->array[li]->N;
    }
    s->impl->na = (NeuronAddress*) malloc(num_neurons * sizeof(NeuronAddress));
    s->impl->num_neurons = num_neurons;
    size_t iter = 0;
    for(size_t li=0; li<s->layers->size; li++) {
        for(size_t ni=0; ni<s->layers->array[li]->N; ni++) {
            s->impl->na[iter].n_id = ni;
            s->impl->na[iter].layer_id = li;
            ++iter;
        }
    }
}

void configureRewardModulation(Sim *s) {
    for(size_t i=0; i<s->rt->pattern_classes->size; i++) {
        int index = -1;
        for(size_t ci=0; ci < s->rt->uniq_classes->size; ci++) {
            if(s->rt->uniq_classes->array[ci] == (int)s->rt->pattern_classes->array[i]) {
                index = ci;
            }
        }
        if((index<0) || (s->rt->uniq_classes->size == 0)) {
            TEMPLATE(insertVector,int)(s->rt->uniq_classes, (int)s->rt->pattern_classes->array[i]);
        }
    }

    sortIntVector(s->rt->uniq_classes);

    for(size_t i=0; i<s->rt->pattern_classes->size; i++) {
        int index = -1;
        for(size_t ci=0; ci < s->rt->uniq_classes->size; ci++) {
            if(s->rt->uniq_classes->array[ci] == (int)s->rt->pattern_classes->array[i]) {
                index = ci;
            }
        }
        assert(index >= 0);
        TEMPLATE(insertVector,ind)(s->rt->classes_indices_train, index);
    } 
    if(s->layers->array[ s->layers->size-1 ]->N % s->rt->uniq_classes->size != 0) {
        printf("Need last layer be a delimiter of number of unique classes in input train spikes\n");
        exit(1);
    }

}

