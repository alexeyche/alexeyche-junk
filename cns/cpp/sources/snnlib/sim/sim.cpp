
#include "sim.h"

#include <pthread.h>

#include <snnlib/config/factory.h>
#include <snnlib/layers/srm_neuron.h>

Sim::Sim(const Constants &c, size_t _jobs) : jobs(_jobs), sc(c.sim_conf) {
    input_neurons_count = 0;
    for(size_t l_id = 0; l_id < sc.input_layers_conf.size(); l_id++) {
        LayerConf conf = sc.input_layers_conf[l_id];
        Layer *l = Factory::inst().createLayer(conf.size, conf.nconf, c, &rg);
        layers.push_back(l);
        input_neurons_count += l->N;
    }
    input_layers_count = sc.input_layers_conf.size();


    net_neurons_count = 0;
    for(size_t l_id = 0; l_id < sc.net_layers_conf.size(); l_id++) {
        LayerConf conf = sc.net_layers_conf[l_id];
        Layer *l = Factory::inst().createLayer(conf.size, conf.nconf, c, &rg);
        layers.push_back(l);
        net_neurons_count += l->N;
    }


    for(auto it=sc.conn_map.begin(); it != sc.conn_map.end(); ++it) {
        pair<size_t, size_t> l_ids = it->first;

        Layer *pre = nullptr, *post = nullptr;
        if(l_ids.first < layers.size()) {
            pre = layers[l_ids.first];
        }
        if(l_ids.second < layers.size()) {
            post = layers[l_ids.second];
        }

        if((!pre)||(!post)) {
            cerr << "Can't deduce layers from ids " << l_ids.first << "-" << l_ids.second << "\n";
            terminate();
        }
        vector<ConnectionConf> conns = it->second;
        for(auto it=conns.begin(); it != conns.end(); ++it) {
            pre->connect(*post, *it, c);
        }
    }

    net.init(this);
}



void Sim::precalculateInputLayerSpikes() {
    // RunTimeDelegates rtd;
    // for(size_t li=0; li<input_layers.size(); li++) {
    //     Layer *l = input_layers[li];
    //     for(size_t ni=0; ni<l->N; ni++) {
    //         l->neurons[ni]->provideDelegates(rtd);
    //     }
    // }


    if(input_ts.size() == 0) {
        cerr << "Need set input time series to precalculate spikes\n";
        terminate();
    }
    
    rg.setDt(sc.ts_map_conf.dt);
    for(double t=0; t<=input_ts.Tmax; t += rg.Dt()) {
        const double &x = input_ts.pop_value();

        for(size_t li=0; li<input_layers_count; li++) {
            Layer *l = layers[li];
            for(size_t ni=0; ni<l->N; ni++) {

                l->neurons[ni]->attachCurrent(x);
                l->neurons[ni]->calculateProbability();
                l->neurons[ni]->calculateDynamics();

                if(l->neurons[ni]->fired) {
                    net.propagateSpike(l->neurons[ni]->id, t);
                    l->neurons[ni]->fired = 0;
                }
            }
        }
    }
}

pthread_barrier_t barrier;

void* Sim::runWorker(void *context) {
    Sim::SimWorker *sw = static_cast<Sim::SimWorker*>(context);
    Sim *s = sw->s;

    for(double t=0; t<=sw->s->input_ts.Tmax; t += s->rg.Dt()) {
        for(size_t li=s->input_layers_count; li<s->layers.size(); li++) {
            Layer *l = s->layers[li];
            for(size_t ni=sw->first; ni<sw->last; ni++) {
                while(const SynSpike* sp = s->net.getSpike(l->neurons[ni]->id, t)) {
                    l->neurons[ni]->propagateSynSpike(sp);
                }
                
                l->neurons[ni]->calculateProbability();
                l->neurons[ni]->calculateDynamics();

                if(l->neurons[ni]->fired) {
                    s->net.propagateSpike(l->neurons[ni]->id, t);
                    l->neurons[ni]->fired = 0;
                }
            }
        }
    }
}

void Sim::run() {
    precalculateInputLayerSpikes();
    net.dispathSpikes(net.spikes_list);
    rg.setDt(sc.sim_run_c.dt);
    
    size_t num_neurons = net_neurons_count;

    if(num_neurons < jobs) {
        jobs = num_neurons;
    }
    pthread_t *threads = new pthread_t[jobs];
    Sim::SimWorker *workers = new Sim::SimWorker[jobs];

    for(size_t ti=0; ti < jobs; ti++) {
        workers[ti].thread_id = ti;
        workers[ti].s = this;
        int neuron_per_thread = (num_neurons + jobs - 1) / jobs;
        workers[ti].first = min( ti    * neuron_per_thread, num_neurons );
        workers[ti].last  = min( (ti+1) * neuron_per_thread, num_neurons );
    }

    pthread_attr_t attr;
    P( pthread_attr_init( &attr ) );
    P( pthread_barrier_init( &barrier, NULL, jobs ) );
    for( int i = 1; i < jobs; i++ )  {
        P( pthread_create( &threads[i], &attr, &Sim::runWorker, &workers[i]) );
    }
    runWorker(&workers[0]);

    delete []workers;
    delete []threads;
}
