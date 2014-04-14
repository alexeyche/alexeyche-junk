
#include "sim.h"

void resetQueue(NetSim *ns, SimRuntime *sr, const size_t *n_id) {
//    size_t *spike_it = &sr->spikes_iter->array[ *n_id ];
//    while(*spike_it < ns->spikes_queue[ *n_id ]->size) {
//        *spike_it = *spike_it + 1;
//    }
    sr->spikes_iter->array[ *n_id ] = 0;
    TEMPLATE(deleteVector,SynSpike)(ns->spikes_queue[ *n_id ]);
    TEMPLATE(createVector,SynSpike)(ns->spikes_queue[ *n_id ]);
}


const SynSpike* getInputSpike(double t, const size_t *n_id, NetSim *ns, SimRuntime *sr, const Constants *c) {
    size_t *spike_it = &sr->input_spikes_iter->array[ *n_id ];
    if(*spike_it < ns->input_spikes_queue[ *n_id ]->size) {
        const SynSpike *sp = &ns->input_spikes_queue[ *n_id ]->array[ *spike_it ];
        if(sp->t - t < 0) {
            printf("We missing an input spike %f in %zu at %f. Something wrong (%f). Need sorted queue\n", sp->t, *n_id, t, sp->t - t);
            exit(1);
        }
        if(sp->t < t+c->dt) {
           *spike_it += 1; 
           return(sp);
        }
    }
    spike_it = &sr->spikes_iter->array[ *n_id ];
    pthread_spin_lock(&spinlocks[ *n_id ]);
    if(*spike_it < ns->spikes_queue[ *n_id ]->size) {
        const SynSpike *sp = &ns->spikes_queue[ *n_id ]->array[ *spike_it ]; 
        if(sp->t - t < 0) {
            printf("We missing a net spike %f in %zu at %f. Something wrong(%f). Need sorted queue\n", sp->t, *n_id, t, sp->t - t);
            exit(1);
        }
        if(sp->t <= t+c->dt) {
           *spike_it += 1; 
           pthread_spin_unlock(&spinlocks[*n_id]);
           return(sp);
        }    
    }
    pthread_spin_unlock(&spinlocks[*n_id]);
    return(NULL);
}



void runSim(Sim *s) {
    configureSimAttr(s);
    
    spinlocks = (pthread_spinlock_t*)malloc( s->net_size * sizeof(pthread_spinlock_t));
    for(size_t ni=0; ni<s->net_size; ni++) {
        pthread_spin_init(&spinlocks[ni], 0); // net sim spinlock
    }
    
    pthread_t *threads = (pthread_t *) malloc( s->nthreads * sizeof( pthread_t ) );
    SimWorker *workers = (SimWorker*) malloc( s->nthreads * sizeof(SimWorker) );
    for(size_t ti=0; ti < s->nthreads; ti++) {
        workers[ti].thread_id = ti;
        workers[ti].s = s;
    }
    
    pthread_attr_t attr;
    P( pthread_attr_init( &attr ) );
    P( pthread_barrier_init( &barrier, NULL, s->nthreads ) );
    for( int i = 1; i < s->nthreads; i++ )  {
        P( pthread_create( &threads[i], &attr, simRunRoutine,  &workers[i]) );
    }
    simRunRoutine(&workers[0]);

    free(workers);
    free(threads);
    for(size_t ni=0; ni<s->net_size; ni++) {
        pthread_spin_destroy(&spinlocks[ni]);
    }
}

void* simRunRoutine(void *args) {
    SimWorker *sw = (SimWorker*)args;
    Sim *s = sw->s;

    int neuron_per_thread = (s->num_neurons + s->nthreads - 1) / s->nthreads;
    int first = min(  sw->thread_id    * neuron_per_thread, s->num_neurons );
    int last  = min( (sw->thread_id+1) * neuron_per_thread, s->num_neurons );

    for(double t=0; t< s->rt->Tmax; t+=s->c->dt) {
        for(size_t na_i=first; na_i<last; na_i++) {
            simulateNeuron(s, &s->na[na_i].layer_id, &s->na[na_i].n_id, t, s->c);
        }
        bool we_did_reset = false;
        if(s->rt->reset_timeline->size > s->rt->timeline_iter) {
            if(s->rt->reset_timeline->array[ s->rt->timeline_iter ] <= t) {
                we_did_reset = true;
                for(size_t na_i=first; na_i<last; na_i++) {
                    SRMLayer *l = s->layers->array[s->na[na_i].layer_id];
                    resetSRMLayerNeuron(l, &s->na[na_i].n_id);
                    resetQueue(s->ns, s->rt, &l->ids[s->na[na_i].n_id]);
                }

            }
        }
        pthread_barrier_wait( &barrier );
        if(we_did_reset) {
            if(sw->thread_id == 0) {
                ++s->rt->timeline_iter;
            }
            pthread_barrier_wait( &barrier );
        }
    }
    return(NULL);
}

void simulateNeuron(Sim *s, const size_t *layer_id, const size_t *n_id, double t,  const Constants *c) {
    const SynSpike *sp;
    SRMLayer *l = s->layers->array[*layer_id];
    while( (sp = getInputSpike(t, &l->ids[*n_id], s->ns, s->rt,  c)) != NULL) {
//        printf("got input spike on %zu:%zu at %f in syn %zu (%f)\n", *layer_id, *n_id, t, sp->syn_id, sp->t);
        propagateSpikeSRMLayer(l, n_id, sp, c);
    }
    simulateSRMLayerNeuron(l, n_id, c);
    if(l->fired[*n_id] == 1) {
        propagateSpikeNetSim(s->ns, s->rt, &l->ids[*n_id], t + l->axon_del[*n_id]);
        l->fired[*n_id] = 0;
    }
}


