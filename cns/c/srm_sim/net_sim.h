#ifndef NET_SIM_H
#define NET_SIM_H

#include <spikes_list.h>
#include <layer.h>

typedef struct {
    size_t n_id;
    size_t syn_id;
} Conn;

#include <templates_clean.h>
#define T Conn
#include <util/util_vector_tmpl.h>

#include <templates_clean.h>
#define T SynSpike
#include <util/util_vector_tmpl.h>



typedef struct {
    ConnVector **conn_map;
    SpikesList *net;    
    SynSpikeVector **input_spikes_queue;
    size_t size;
} NetSim;

NetSim* createNetSim();
void deallocNetSim(NetSim *ns);
void allocNetSim(NetSim *ns, size_t net_size);
void deleteNetSim(NetSim *ns);
void propagateInputSpikesNetSim(NetSim *ns, SpikesList *sl);
void printInputSpikesQueue(NetSim *ns);
void configureConnMapNetSim(NetSim *ns, pSRMLayerVector *l);
void propagateSpikeNetSim(NetSim *ns, size_t ni, double t);

#endif
