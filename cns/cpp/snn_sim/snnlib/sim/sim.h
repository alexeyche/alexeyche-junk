#pragma once


#include <snnlib/layers/layer.h>
#include <snnlib/config/factory.h>

class Sim: public Printable {
public: 
    Sim(const Constants &c) : sc(c.sim_conf) {
        for(size_t l_id=0; l_id < sc.input_layers_conf.size(); l_id++) {
            InputLayersConf conf = sc.input_layers_conf[l_id];
            LayerObj* l = factory.createInputLayer(conf.type, l_id, conf.size, c[conf.type]);
            input_layers.push_back(l);
        }
        for(size_t l_id=0; l_id < sc.net_layers_conf.size(); l_id++) {
            NetLayersConf conf = sc.net_layers_conf[l_id];
            ActFunc *af = factory.createActFunc(conf.act_func, c[conf.act_func]);
            LearningRule *lr = factory.createLearningRule(conf.learning_rule, c[conf.learning_rule]);
            LayerObj* l = factory.createNetLayer(conf.type, l_id, conf.size, c[conf.type], af, lr);
            layers.push_back(l);
        }
    }
    ~Sim() {
        input_layers.clear();
        layers.clear();
    }
    void print(std::ostream& str) const {
        for(auto it=input_layers.begin(); it!=input_layers.end(); ++it) {
            str << **it;
        }
        for(auto it=layers.begin(); it!=layers.end(); ++it) {
            str << **it;
        }
    }
    
    vector<LayerObj*> input_layers;
    vector<LayerObj*> layers;

    const SimConfiguration &sc;
};


