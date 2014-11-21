
#include "sim.h"

#include <thread>
#include <mutex>

#include <snnlib/config/factory.h>
#include <snnlib/layers/srm_neuron.h>

Sim::Sim(const Constants &c, size_t _jobs) : jobs(_jobs), sc(c.sim_conf) {
    input_neurons_count = 0;
    for(size_t l_id = 0; l_id < sc.input_layers_conf.size(); l_id++) {
        LayerConf conf = sc.input_layers_conf[l_id];
        Layer *l = Factory::inst().createLayer(conf.size, conf.nconf, c, &rg);
        layers.push_back(l);
        input_neurons_count += l->N;
        for(size_t ni=0; ni<l->N; ni++) {
            Neuron *n = l->neurons[ni];
            sim_neurons.push_back( SimNeuron(n, layers.size()-1, ni) );
        }
    }
    input_layers_count = sc.input_layers_conf.size();


    net_neurons_count = 0;
    for(size_t l_id = 0; l_id < sc.net_layers_conf.size(); l_id++) {
        LayerConf conf = sc.net_layers_conf[l_id];
        Layer *l = Factory::inst().createLayer(conf.size, conf.nconf, c, &rg);
        layers.push_back(l);
        net_neurons_count += l->N;
        for(size_t ni=0; ni<l->N; ni++) {
            Neuron *n = l->neurons[ni];
            sim_neurons.push_back( SimNeuron(n, layers.size()-1, ni) );
        }
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



    // RunTimeDelegates rtd;
    // for(size_t li=0; li<input_layers.size(); li++) {
    //     Layer *l = input_layers[li];
    //     for(size_t ni=0; ni<l->N; ni++) {
    //         l->neurons[ni]->provideDelegates(rtd);
    //     }
    // }





pthread_barrier_t barrier;

void Sim::runWorker(size_t thread_id, Sim *s, size_t neuron_first_id, size_t neuron_last_id) {
    cout << "thread " << thread_id << " started on neurons " << neuron_first_id << "-" << neuron_last_id << "\n";

    for(double t=0; t<=s->input_ts.Tmax; t += s->rg.Dt()) {
        double x = 0.0;
        if(neuron_last_id <= s->input_neurons_count){
            if(s->input_ts.size()>0) {
                x = s->input_ts.top_value();
            }
            if(thread_id == 0) {
                s->input_ts.pop_value();
            }
        }
        for(size_t ni=neuron_first_id; ni<neuron_last_id; ni++) {
            Neuron *n = s->sim_neurons[ni].n;
            if(ni<s->input_neurons_count) {
                n->attachCurrent(x);
            }

            while(const SynSpike *sp = s->net.getSpike(ni, t)) {
                n->propagateSynSpike(sp);
            }
            n->calculateProbability();
            n->calculateDynamics();

            if(n->fired) {
                s->net.propagateSpike(n->id, t+s->rg.Dt());
                n->fired = 0;
            }

        }
        pthread_barrier_wait( &barrier );

    }
    cout << "thread " << thread_id << " exited\n";
}


void Sim::runSimOnSubset(size_t left_neuron_id, size_t right_neuron_id) {
    assert(left_neuron_id<right_neuron_id);
    size_t num_neurons = right_neuron_id-left_neuron_id;

    if(num_neurons < jobs) {
        jobs = num_neurons;
    }
    P( pthread_barrier_init( &barrier, NULL, jobs ) );

    std::vector<std::thread> v;
    for (size_t ji = 0; ji < jobs; ++ji) {
        int neuron_per_thread = (num_neurons + jobs - 1) / jobs;
        size_t first = min( ji    * neuron_per_thread, num_neurons ) + left_neuron_id;
        size_t last  = min( (ji+1) * neuron_per_thread, num_neurons ) + left_neuron_id;

        v.emplace_back(Sim::runWorker, ji, this, first, last);
    }
    for (auto& t : v) {
        t.join();
    }
    P( pthread_barrier_destroy(&barrier) );
}

void Sim::run() {
    if(input_ts.size() == 0) {
        cerr << "Need set input time series to precalculate spikes\n";
        terminate();
    }
    // rg.setDt(sc.ts_map_conf.dt);
    // runSimOnSubset(0, input_neurons_count);
    // net.dispathInputSpikes(net.spikes_list);

    rg.setDt(sc.sim_run_c.dt);
    //runSimOnSubset(input_neurons_count, input_neurons_count+net_neurons_count);
    runSimOnSubset(0, input_neurons_count+net_neurons_count);
}
