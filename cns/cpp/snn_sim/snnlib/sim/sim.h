#pragma once


#include <snnlib/layers/layer.h>
#include <snnlib/config/factory.h>

class Sim: public Printable {
public: 
    Sim(const Constants &c) : sc(c.sim_conf) {
        const ConstObj * f = c.act_funcs.at("Determ").get();
        auto af = factory.createActFunc("Determ", f);
        auto lr = factory.createLearningRule("OptimalStdp", c.learning_rules.at("OptimalStdp").get());
        LayerObj *l = factory.createLayerObj("SRMLayer", 0, 100, c.net_layers.at("SRMLayer").get(), af, lr);
        cout << *(Layer<SRMNeuron>*)(l);
    }
    
    void print(std::ostream& str) const {
        for(auto it=layers.begin(); it!=input_layers.end(); ++it) {
            str << **it;
        }
        for(auto it=layers.begin(); it!=layers.end(); ++it) {
            str << **it;
        }
    }
    
    vector< unique_ptr<LayerObj> > input_layers;
    vector< unique_ptr<LayerObj> > layers;

    const SimConfiguration &sc;
};


