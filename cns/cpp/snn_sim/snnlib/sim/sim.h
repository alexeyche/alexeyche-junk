#pragma once


#include <snnlib/layers/layer.h>

class Sim: public Printable {
public: 
    Sim(const Constants &c) : sc(c.sim_conf) {
        for(size_t i=0; i<sc.input_layers_conf.size(); i++) {
//            InputLayersConf conf = sc.input_layers_conf[i];
//            factory.createLayerObj(conf.type
        }
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


