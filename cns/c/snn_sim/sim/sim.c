
#include "sim.h"

pthread_barrier_t barrier;

Sim* createSim(size_t nthreads, unsigned char stat_level, Constants *c) {
    Sim *s = (Sim*) malloc(sizeof(Sim));
    s->ns = createNetSim();
    s->layers = TEMPLATE(createVector,pLayer)(); 
    s->rt = createRuntime();
    s->ctx = (SimContext*) malloc(sizeof(SimContext));
    s->ctx->c = c;
    s->ctx->global_reward = 0;
    s->ctx->mean_global_reward = 0;
    s->ctx->stat_level = stat_level;
    if(s->ctx->stat_level > 0) {
        s->ctx->stat_global_reward = TEMPLATE(createVector,double)();
    }
    s->ctx->actual_running_time = 0.0;
    s->impl = (SimImpl*) malloc(sizeof(SimImpl));
    s->impl->nthreads = nthreads;
    s->impl->net_size=0;
    s->impl->na = NULL;
    s->impl->num_neurons = 0;
    return(s);
}

void simSetInputSpikes(Sim *s, SpikesList *sl) {
    if(s->rt) {
        deleteRuntime(s->rt);
        s->rt = createRuntime();
    }
    configureNetSpikesSim(s, s->ctx->c);

    propagateInputSpikesNetSim(s, sl);

    double Tmax = 0;
    for(size_t ni=0; ni<s->ns->net->size; ni++) {
        if(s->ns->net->list[ni]->size>0) {
            if(s->ns->net->list[ni]->array[ s->ns->net->list[ni]->size-1 ]> Tmax) {
                Tmax = s->ns->net->list[ni]->array[ s->ns->net->list[ni]->size-1 ];
            }
        }
    }   
    s->rt->Tmax = Tmax + 100;
}

void simSetInputSpikePatterns(Sim *s, SpikePatternsList *spl) {
    simSetInputSpikes(s, spl->sl);
    s->rt->reset_timeline = TEMPLATE(copyVector,double)(spl->timeline);
    s->rt->pattern_classes = TEMPLATE(copyVector,double)(spl->pattern_classes);
}


void appendLayerSim(Sim *s, LayerPoisson *l) {
    TEMPLATE(insertVector,pLayer)(s->layers, l);
    l->id = s->layers->size - 1;
}

void deleteSim(Sim *s) {
    TEMPLATE(deleteVector,pLayer)(s->layers);
    deleteNetSim(s->ns);
    deleteRuntime(s->rt);
    if(s->ctx->stat_level > 1) {
        TEMPLATE(deleteVector,double)(s->ctx->stat_global_reward);
    }
    free(s->ctx);
    free(s->impl);
    free(s);
}

size_t getLayerIdOfNeuron(Sim *s, size_t n_id) {
    for(size_t li=0; li<s->layers->size; li++) {
        LayerPoisson *l = s->layers->array[li];
        assert(l->N);
        if((l->ids[0] <= n_id) && (l->ids[l->N-1] >= n_id)) {
            return(l->id);
        }
    }
    printf("Can't find layer id for neuron with id %zu\n", n_id);
    exit(1);
}


const SynSpike* getInputSpike(Sim *s, const size_t *layer_id, const size_t *n_id, const double *t) {
    const size_t gn_id = getGlobalId(s->layers->array[ *layer_id ], n_id);
    const SimContext *ctx = s->ctx;
    const Constants *c = ctx->c;
    if(s->ns->input_spikes_queue[ gn_id ]->current != NULL) {
        const SynSpike *sp = &s->ns->input_spikes_queue[ gn_id ]->current->value;
        if(sp->t - *t < 0) {
            printf("We missing an input spike %f in %zu at %f. Something wrong (%f). Need sorted queue\n", sp->t, gn_id, *t, sp->t - *t);
            exit(1);
        }
        if(sp->t <= *t+c->dt) {
            s->ns->input_spikes_queue[ gn_id ]->current = s->ns->input_spikes_queue[ gn_id ]->current->next;
            return(sp);
        }
    }
    pthread_spin_lock(&spinlocks[ gn_id ]);
    if(s->ns->spikes_queue[ gn_id ]->current != NULL) {
        const SynSpike *sp = &s->ns->spikes_queue[ gn_id ]->current->value;
        if(sp->t - *t < 0) {
            printf("We missing net spike %f in %zu at %f. Something wrong (%f). Need sorted queue\n", sp->t, gn_id, *t, sp->t - *t);
            exit(1);
        }
        if(sp->t <= *t+c->dt) {
            s->ns->spikes_queue[ gn_id ]->current = s->ns->spikes_queue[ gn_id ]->current->next;
            pthread_spin_unlock(&spinlocks[gn_id]);
            return(sp);
        }
    }
    pthread_spin_unlock(&spinlocks[gn_id]);
    return(NULL);
}



void runSim(Sim *s) {
    configureSimImpl(s);

    spinlocks = (pthread_spinlock_t*)malloc( s->impl->net_size * sizeof(pthread_spinlock_t));
    for(size_t ni=0; ni<s->impl->net_size; ni++) {
        pthread_spin_init(&spinlocks[ni], 0); // net sim spinlock
    }
    
    pthread_t *threads = (pthread_t *) malloc( s->impl->nthreads * sizeof( pthread_t ) );
    SimWorker *workers = (SimWorker*) malloc( s->impl->nthreads * sizeof(SimWorker) );
    for(size_t ti=0; ti < s->impl->nthreads; ti++) {
        workers[ti].thread_id = ti;
        workers[ti].s = s;
        int neuron_per_thread = (s->impl->num_neurons + s->impl->nthreads - 1) / s->impl->nthreads;
        workers[ti].first = min( ti    * neuron_per_thread, s->impl->num_neurons );
        workers[ti].last  = min( (ti+1) * neuron_per_thread, s->impl->num_neurons );
    }
    
    pthread_attr_t attr;
    P( pthread_attr_init( &attr ) );
    P( pthread_barrier_init( &barrier, NULL, s->impl->nthreads ) );
    for( int i = 1; i < s->impl->nthreads; i++ )  {
        P( pthread_create( &threads[i], &attr, simRunRoutine,  &workers[i]) );
    }
    simRunRoutine(&workers[0]);

    free(workers);
    free(threads);
    for(size_t ni=0; ni<s->impl->net_size; ni++) {
        pthread_spin_destroy(&spinlocks[ni]);
    }
//    if(s->c->reinforcement) {
//        pthread_spin_destroy(global_reward_spinlock);
//    }
}

void* simRunRoutine(void *args) {
    SimWorker *sw = (SimWorker*)args;
    Sim *s = sw->s;
    SimContext *ctx = s->ctx;
    const Constants *c = ctx->c; 
    SimImpl* impl = s->impl;
    
    bool need_sync = false;
    for(size_t na_i=sw->first; na_i<sw->last; na_i++) {
        LayerPoisson *l = s->layers->array[ impl->na[na_i].layer_id ];
        if(l->need_steps_sync) {
            need_sync = true;
        }
    }
    
    for(double t=0; t< s->rt->Tmax; t+=c->dt) {
//        printf("t: %f\n",ctx->t);
        for(size_t na_i=sw->first; na_i<sw->last; na_i++) {
            const size_t *layer_id = &impl->na[na_i].layer_id;
            const size_t *n_id = &impl->na[na_i].n_id;

            LayerPoisson *l = s->layers->array[*layer_id];

            const SynSpike *sp;
            while( (sp = getInputSpike(s, layer_id, n_id, &t)) != NULL) {
                //printf("got input spike on %zu:%zu at %f in syn %zu (%f)\n", *layer_id, *n_id, t, sp->syn_id, sp->t);
                l->propagateSpike(l, n_id, sp, s->ctx);
            }

            l->calculateProbability(l, n_id, s->ctx);

            if(!need_sync) {
                l->calculateSpike(l, n_id, s->ctx);
                l->calculateDynamics(l, n_id, s->ctx);
                if(l->fired[*n_id] == 1) {
                    propagateSpikeNetSim(s, l, &l->ids[*n_id], t);
                    l->fired[*n_id] = 0;
                }
            }
        }
        pthread_barrier_wait( &barrier );

        if(need_sync) {
            for(size_t na_i=sw->first; na_i<sw->last; na_i++) {
                const size_t *layer_id = &impl->na[na_i].layer_id;
                const size_t *n_id = &impl->na[na_i].n_id;

                LayerPoisson *l = s->layers->array[*layer_id];

                l->calculateSpike(l, n_id, s->ctx);
            }
            pthread_barrier_wait( &barrier );
            for(size_t na_i=sw->first; na_i<sw->last; na_i++) {
                const size_t *layer_id = &impl->na[na_i].layer_id;
                const size_t *n_id = &impl->na[na_i].n_id;

                LayerPoisson *l = s->layers->array[*layer_id];

                l->calculateDynamics(l, n_id, s->ctx);
                if(l->fired[*n_id] == 1) {
                    propagateSpikeNetSim(s, l, &l->ids[*n_id], t);
                    l->fired[*n_id] = 0;
                }
            }
            pthread_barrier_wait( &barrier );
        }
    }
    if(sw->thread_id == 0) {
        s->ctx->actual_running_time += s->rt->Tmax;
    }
    return(NULL);
}

void simulateNeuron(Sim *s, const size_t *layer_id, const size_t *n_id, const double *t) {
    const Constants *c = s->ctx->c;

}


