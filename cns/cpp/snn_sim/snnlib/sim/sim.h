#pragma once


#include <snnlib/layers/layer.h>

class Sim: public Printable {
public: 
    Sim(const Constants &c) : sc(c.sim_conf) {
        for(size_t i=0; i<sc.input_sizes.size(); i++) {
            string input_layer_type = sc.input_layers[i];
            //c.input_layers[input_layer_type]            
        }
    }
    
    void print(std::ostream& str) const {
        for(auto it=layers.begin(); it!=layers.end(); ++it) {
            str << **it;
        }
    }
    
    vector< unique_ptr<LayerObj> > input_layers;
    vector< unique_ptr<LayerObj> > layers;
    const SimConfiguration &sc;
};


