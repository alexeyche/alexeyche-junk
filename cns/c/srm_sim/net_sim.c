
#include "net_sim.h"

#include <templates_clean.h>
#define T Conn
#include <util/util_vector_tmpl.c>

#include <templates_clean.h>
#define T SynSpike
#include <util/util_vector_tmpl.c>




NetSim* createNetSim() {
    NetSim *ns = (NetSim*) malloc(sizeof(NetSim)); 
    ns->size=0;
    return(ns);
}

void deallocNetSim(NetSim *ns) {
    deleteSpikesList(ns->net);
    for(size_t sp_i=0; sp_i < ns->size; sp_i++) { 
        TEMPLATE(deleteVector,Conn)(ns->conn_map[sp_i]);
        TEMPLATE(deleteVector,SynSpike)(ns->input_spikes_queue[sp_i]);
        TEMPLATE(deleteVector,SynSpike)(ns->spikes_queue[sp_i]);
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
    ns->input_spikes_queue = (SynSpikeVector**) malloc(  net_size*sizeof(SynSpikeVector*));
    ns->spikes_queue = (SynSpikeVector**) malloc(  net_size*sizeof(SynSpikeVector*));
    for(size_t sp_i=0; sp_i < net_size; sp_i++) {
        ns->conn_map[sp_i] = TEMPLATE(createVector,Conn)();
        ns->input_spikes_queue[sp_i] = TEMPLATE(createVector,SynSpike)();
        ns->spikes_queue[sp_i] = TEMPLATE(createVector,SynSpike)();
    }
}


void deleteNetSim(NetSim *ns) {
    if(ns->size > 0) {
        deallocNetSim(ns);
    }
    free(ns);
}

void configureConnMapNetSim(NetSim *ns, pSRMLayerVector *l) {
    for(size_t li=0; li< l->size; li++) {
        for(size_t ni=0; ni < l->array[li]->N; ni++) {
            for(size_t con_i=0; con_i < l->array[li]->nconn[ni]; con_i++) {
                Conn conseq;
                conseq.n_id = l->array[li]->ids[ni];
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

void propagateInputSpikesNetSim(NetSim *ns, SpikesList *sl) {
    assert(ns->size >= sl->size);
    for(size_t ni=0; ni< sl->size; ni++) {
        for(size_t con_i=0; con_i < ns->conn_map[ni]->size; con_i++) {
            for(size_t sp_i=0; sp_i < sl->list[ni]->size; sp_i++) {
                SynSpike sp;
                sp.n_id = ni;
                sp.syn_id = ns->conn_map[ni]->array[con_i].syn_id;
                sp.t = sl->list[ni]->array[sp_i];
                TEMPLATE(insertVector,SynSpike)(ns->input_spikes_queue[ ns->conn_map[ni]->array[con_i].n_id ], sp);
            }
        }
    }
    for(size_t ni=0; ni< ns->size; ni++) {
        qsort(ns->input_spikes_queue[ni]->array, ns->input_spikes_queue[ni]->size, sizeof(SynSpike), compSynSpike);
    }
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
            printf("%zu:%zu, ", ns->conn_map[ni]->array[con_i].n_id, ns->conn_map[ni]->array[con_i].syn_id);
        }
        printf("\n");
    }
}

void printInputSpikesQueue(NetSim *ns) {
    for(size_t ni=0; ni<ns->size; ni++) {
        printf("%zu: ", ni);
        for(size_t inp_i=0; inp_i < ns->input_spikes_queue[ni]->size; inp_i++) {
            const SynSpike *ss = &ns->input_spikes_queue[ni]->array[inp_i];
            printf("%f:%zu:%zu, ", ss->t, ss->n_id, ss->syn_id);
        }
        printf("\n");
    }
}

#define SORT_LIM 30
#define MINIMAL_DELAY 1
void propagateSpikeNetSim(NetSim *ns, const size_t *ni, const double *t) {
    TEMPLATE(insertVector,double)(ns->net->list[*ni], *t);
    for(size_t con_i=0; con_i < ns->conn_map[*ni]->size; con_i++) {
        SynSpike sp;        
        sp.n_id = *ni;
        sp.syn_id = ns->conn_map[*ni]->array[con_i].syn_id;
        sp.t = *t + MINIMAL_DELAY;
        TEMPLATE(insertVector,SynSpike)(ns->spikes_queue[ ns->conn_map[*ni]->array[con_i].n_id ], sp);
        size_t size_to_sort = ns->spikes_queue[*ni]->size;
        size_t offset = 0;
        if(size_to_sort > SORT_LIM) {
            size_to_sort = SORT_LIM;
            offset = ns->spikes_queue[*ni]->size-size_to_sort;
        }
        qsort(ns->spikes_queue[*ni]->array+offset, size_to_sort, sizeof(SynSpike), compSynSpike);
//        printf("queue of %zu: ", ns->conn_map[*ni]->array[con_i].n_id);
//        for(size_t i=0; i<ns->spikes_queue[ ns->conn_map[*ni]->array[con_i].n_id ]->size; i++) {
//            printf("%f:%zu ", ns->spikes_queue[ ns->conn_map[*ni]->array[con_i].n_id ]->array[i].t,  \
//                              ns->spikes_queue[ ns->conn_map[*ni]->array[con_i].n_id ]->array[i].syn_id);
//        }
//        printf("\n");
    }
}


