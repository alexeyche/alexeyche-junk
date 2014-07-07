#ifndef SIM_NETSIM_H
#define SIM_NETSIM_H


// Net sim

typedef struct {
    size_t l_id;
    size_t n_id;
    size_t syn_id;
} Conn;

#include <util/templates_clean.h>
#define T Conn
#include <util/util_vector_tmpl.h>

#include <util/templates_clean.h>
#define T SynSpike
#include <util/util_vector_tmpl.h>

#include <util/templates_clean.h>
#define T SynSpike
#include <util/util_dlink_list_tmpl.h>


pthread_spinlock_t *spinlocks;

typedef struct {
    ConnVector **conn_map;
    SpikesList *net;    
    SynSpikeLList **input_spikes_queue;
    SynSpikeLList **spikes_queue;
    size_t size;
} NetSim;

struct Sim;

NetSim* createNetSim();
void deallocNetSim(NetSim *ns);
void allocNetSim(NetSim *ns, size_t net_size);
void deleteNetSim(NetSim *ns);
void propagateInputSpikesNetSim(struct Sim *s, SpikesList *sl);
void printInputSpikesQueue(NetSim *ns);
void configureConnMapNetSim(NetSim *ns, pLayerVector *l);
void propagateSpikeNetSim(struct Sim *s, Layer *l, const size_t *ni, double t);
void printConnMap(NetSim *ns);

#endif
