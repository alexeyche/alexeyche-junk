
#include <sim/sim.h>

#include <util/templates_clean.h>
#define T Conn
#include <util/util_vector_tmpl.c>

#include <util/templates_clean.h>
#define T SynSpike
#include <util/util_vector_tmpl.c>

#include <util/templates_clean.h>
#define T SynSpike
#include <util/util_dlink_list_tmpl.c>




NetSim* createNetSim() {
    NetSim *ns = (NetSim*) malloc(sizeof(NetSim)); 
    ns->size=0;
    return(ns);
}

void deallocNetSim(NetSim *ns) {
    deleteSpikesList(ns->net);
    for(size_t sp_i=0; sp_i < ns->size; sp_i++) { 
        TEMPLATE(deleteVector,Conn)(ns->conn_map[sp_i]);
        TEMPLATE(deleteLList,SynSpike)(ns->input_spikes_queue[sp_i]);
        TEMPLATE(deleteLList,SynSpike)(ns->spikes_queue[sp_i]);
    }
    free(ns->conn_map);
    free(ns->input_spikes_queue);
    free(ns->spikes_queue);
    ns->size = 0;
}



void allocNetSim(NetSim *ns, size_t net_size) {
    if(ns->size > 0) {
        deallocNetSim(ns);    
    }
    ns->size = net_size;
    ns->net = createSpikesList(net_size);
    ns->conn_map = (ConnVector**) malloc( net_size*sizeof(ConnVector*));
    ns->input_spikes_queue = (SynSpikeLList**) malloc(  net_size*sizeof(SynSpikeLList*));
    ns->spikes_queue = (SynSpikeLList**) malloc(  net_size*sizeof(SynSpikeLList*));
    for(size_t sp_i=0; sp_i < net_size; sp_i++) {
        ns->conn_map[sp_i] = TEMPLATE(createVector,Conn)();
        ns->input_spikes_queue[sp_i] = TEMPLATE(createLList,SynSpike)();
        ns->spikes_queue[sp_i] = TEMPLATE(createLList,SynSpike)();
    }
}


void deleteNetSim(NetSim *ns) {
    if(ns->size > 0) {
        deallocNetSim(ns);
    }
    free(ns);
}

void configureConnMapNetSim(NetSim *ns, pLayerVector *l) {
    for(size_t li=0; li< l->size; li++) {
        for(size_t ni=0; ni < l->array[li]->N; ni++) {
            for(size_t con_i=0; con_i < l->array[li]->nconn[ni]; con_i++) {
                Conn conseq;
                conseq.l_id = li;
                conseq.n_id = ni;
                conseq.syn_id = con_i;
                TEMPLATE(insertVector,Conn)( ns->conn_map[ l->array[li]->id_conns[ni][con_i] ], conseq); 
            }
        }
    }
}

int compSynSpike( const void *elem1, const void *elem2 ) {
    SynSpike* s1 = (SynSpike*)elem1;
    SynSpike* s2 = (SynSpike*)elem2;
    if(s1->t > s2->t) return(1);
    if(s1->t < s2->t) return(-1);
    return(0);
}

void propagateInputSpikesNetSim(Sim *s, SpikesList *sl) {
    NetSim *ns = s->ns;
    assert(ns->size >= sl->size);
    SynSpikeVector **input_spikes = (SynSpikeVector**) malloc( ns->size * sizeof(SynSpikeVector*));
    for(size_t ni=0; ni< ns->size; ni++) {
        input_spikes[ni] = TEMPLATE(createVector,SynSpike)();
    }
    
    for(size_t ni=0; ni< sl->size; ni++) {
        for(size_t con_i=0; con_i < ns->conn_map[ni]->size; con_i++) {
            Layer *l = s->layers->array[ ns->conn_map[ni]->array[con_i].l_id ];
            
            for(size_t sp_i=0; sp_i < sl->list[ni]->size; sp_i++) {
                SynSpike sp;
                sp.n_id = ni;
                sp.syn_id = ns->conn_map[ni]->array[con_i].syn_id;
                assert(l);
                sp.t = sl->list[ni]->array[sp_i] + getSynDelay(l, &ns->conn_map[ni]->array[con_i].n_id, &sp.syn_id);
                TEMPLATE(insertVector,SynSpike)(input_spikes[ getGlobalId(l, &ns->conn_map[ni]->array[con_i].n_id) ], sp);
            }
        }
    }
    
    for(size_t ni=0; ni< ns->size; ni++) {
        qsort(input_spikes[ni]->array, input_spikes[ni]->size, sizeof(SynSpike), compSynSpike);
        for(size_t sp_i=0; sp_i < input_spikes[ni]->size; sp_i++) {
            TEMPLATE(addValueLList,SynSpike)(ns->input_spikes_queue[ni], input_spikes[ni]->array[sp_i]);
        }
        ns->input_spikes_queue[ni]->current = ns->input_spikes_queue[ni]->first;
        TEMPLATE(deleteVector,SynSpike)(input_spikes[ni]);
    }
    free(input_spikes);
    
    for(size_t ni=0; ni< sl->size; ni++) {
        for(size_t sp_i=0; sp_i < sl->list[ni]->size; sp_i++) {
            TEMPLATE(insertVector,double)(ns->net->list[ni], sl->list[ni]->array[sp_i]);
        }
    }

}

void printConnMap(NetSim *ns) {
    printf("ConnMap (neurons cause spike in...)\n");
    for(size_t ni=0; ni<ns->size; ni++) {
        printf("%zu: ", ni);
        for(size_t con_i=0; con_i < ns->conn_map[ni]->size; con_i++) {
            printf("%zu:%zu:%zu, ", ns->conn_map[ni]->array[con_i].l_id, ns->conn_map[ni]->array[con_i].n_id, ns->conn_map[ni]->array[con_i].syn_id);
        }
        printf("\n");
    }
}

void printInputSpikesQueue(NetSim *ns) {
    for(size_t ni=0; ni<ns->size; ni++) {
        printf("%zu: ", ni);
        SynSpikeLNode *current_node = ns->input_spikes_queue[ ni ]->current;
        SynSpikeLNode *spike_node;
        while( (spike_node = TEMPLATE(getNextLList,SynSpike)(ns->input_spikes_queue[ ni ]) ) != NULL ) {
            printf("%zu:%zu:%3.3f ", spike_node->value.n_id, spike_node->value.syn_id, spike_node->value.t);
        } 
        ns->input_spikes_queue[ ni ]->current = current_node;
        printf("\n");
    }
}

#define SORT_LIM 10
#define MINIMAL_DELAY 1

void propagateSpikeNetSim(Sim *s, Layer *l, const size_t *ni, double t) {
    NetSim *ns = s->ns;
    TEMPLATE(insertVector,double)(ns->net->list[*ni], t);
    double ax_delay = l->axon_del[getLocalNeuronId(l, ni)];

    for(size_t con_i=0; con_i < ns->conn_map[*ni]->size; con_i++) {
        SynSpike sp;        
        sp.n_id = *ni;
        sp.syn_id = ns->conn_map[*ni]->array[con_i].syn_id;

        Layer *l_cons = s->layers->array[ ns->conn_map[*ni]->array[con_i].l_id ];
        sp.t = t + MINIMAL_DELAY + ax_delay + getSynDelay(l_cons, &ns->conn_map[*ni]->array[con_i].n_id,  &sp.syn_id);
        
        size_t naffect = getGlobalId(l_cons, &ns->conn_map[*ni]->array[con_i].n_id);
        pthread_spin_lock(&spinlocks[naffect]);
        
        SynSpikeLNode *spike_node = ns->spikes_queue[ naffect ]->current;
        if(spike_node) {
            while(spike_node->value.t<sp.t) {
               spike_node = spike_node->next; 
               if(spike_node == NULL) break;
            }
        }
        TEMPLATE(insertBeforeLList,SynSpike)(ns->spikes_queue[ naffect ], spike_node, sp); 
        if(ns->spikes_queue[ naffect ]->current == NULL) {
            ns->spikes_queue[ naffect ]->current = ns->spikes_queue[ naffect ]->last;
        }
        
//        SynSpikeLNode *current_node = ns->spikes_queue[ naffect ]->current;
//        ns->spikes_queue[ naffect ]->current = NULL;
//        printf("%zu: ", naffect);
//        while( (spike_node = TEMPLATE(getNextLList,SynSpike)(ns->spikes_queue[ naffect ]) ) != NULL ) {
//            printf("%zu:%zu:%3.3f |", spike_node->value.n_id, spike_node->value.syn_id, spike_node->value.t);
//        } 
//        printf("\n");
//        ns->spikes_queue[ naffect ]->current = current_node;

        pthread_spin_unlock(&spinlocks[naffect]);
    }
}



