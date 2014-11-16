
#include "sim.h"

#include <snnlib/config/factory.h>
#include <snnlib/layers/srm_layer.h>

Sim::Sim(const Constants &c) : Tmax(0.0), sc(c.sim_conf) {
    size_t l_id = 0;
    for(; l_id < sc.input_layers_conf.size(); l_id++) {
        InputLayersConf conf = sc.input_layers_conf[l_id];
        ActFunc *af = Factory::inst()->createActFunc(conf.act_func, c[conf.act_func]);
        TuningCurve *tc = Factory::inst()->createTuningCurve(conf.tuning_curve, conf.size, c[conf.tuning_curve]);
        Layer* l = Factory::inst()->createInputLayer(conf.type, l_id, conf.size, c[conf.type], af, tc);
        input_layers.push_back(l);
    }
    for(; l_id-input_layers.size() < sc.net_layers_conf.size(); l_id++) {
        NetLayersConf conf = sc.net_layers_conf[l_id-input_layers.size()];
        ActFunc *af = Factory::inst()->createActFunc(conf.act_func, c[conf.act_func]);
        LearningRule *lr = Factory::inst()->createLearningRule(conf.learning_rule, c[conf.learning_rule]);
        Layer* l = Factory::inst()->createLayer(conf.type, l_id, conf.size, c[conf.type], af, lr);
        layers.push_back(l);
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

    net.init(input_layers.size() + layers.size());
}

