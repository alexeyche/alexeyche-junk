
#include "sim.h"

#include <snnlib/config/factory.h>
#include <snnlib/layers/srm_neuron.h>

Sim::Sim(const Constants &c) : sc(c.sim_conf) {
    input_neurons_count = 0;
    for(size_t l_id = 0; l_id < sc.input_layers_conf.size(); l_id++) {
        LayerConf conf = sc.input_layers_conf[l_id];
        Layer *l = Factory::inst().createLayer(conf.size, conf.nconf, c);
        layers.push_back(l);
        input_neurons_count += l->N;
    }
    input_layers_count = sc.input_layers_conf.size();


    net_neurons_count = 0;
    for(size_t l_id = 0; l_id < sc.net_layers_conf.size(); l_id++) {
        LayerConf conf = sc.net_layers_conf[l_id];
        Layer *l = Factory::inst().createLayer(conf.size, conf.nconf, c);
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
    for(double t=0; t<=input_ts.Tmax; t += sc.ts_map_conf.dt) {
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
