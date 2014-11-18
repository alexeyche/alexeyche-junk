
#include "sim.h"

#include <snnlib/config/factory.h>
#include <snnlib/layers/srm_layer.h>

Sim::Sim(const Constants &c) : sc(c.sim_conf) {
    size_t input_neurons_count = 0;
    for(size_t l_id = 0; l_id < sc.input_layers_conf.size(); l_id++) {
        LayerConf conf = sc.input_layers_conf[l_id];
        Layer *l = Factory::inst().createLayer(conf.layer, c[conf.layer], conf.size, conf.nconf, c);
        input_layers.push_back(l);
        input_neurons_count += l->N;
    }
    size_t net_neurons_count = 0;
    for(size_t l_id = 0; l_id < sc.net_layers_conf.size(); l_id++) {
        LayerConf conf = sc.net_layers_conf[l_id];
        Layer *l = Factory::inst().createLayer(conf.layer, c[conf.layer], conf.size, conf.nconf, c);
        layers.push_back(l);
        input_neurons_count += l->N;
    }

    for(auto it=sc.conn_map.begin(); it != sc.conn_map.end(); ++it) {
        pair<size_t, size_t> l_ids = it->first;

        Layer *pre = nullptr, *post = nullptr;
        if(l_ids.first < input_layers.size()) { // deducing pre layer (from input or not)
            pre = input_layers[l_ids.first];
        } else
        if(l_ids.first < layers.size()) {
            pre = layers[l_ids.first];
        }

        if(l_ids.second < input_layers.size()) {
            cerr << "Can't create connection with input layer\n";
            terminate();
        } else
        if(l_ids.second < input_layers.size() + layers.size()) {
            post = layers[l_ids.second - input_layers.size()];
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

    net.init(input_neurons_count, net_neurons_count);
}

