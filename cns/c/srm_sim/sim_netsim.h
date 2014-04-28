#ifndef SIM_NETSIM_H
#define SIM_NETSIM_H


// Net sim

typedef struct {
    size_t l_id;
    size_t n_id;
    size_t syn_id;
} Conn;

#include <templates_clean.h>
#define T Conn
#include <util/util_vector_tmpl.h>

#include <templates_clean.h>
#define T SynSpike
#include <util/util_vector_tmpl.h>

#include <templates_clean.h>
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


#endif
