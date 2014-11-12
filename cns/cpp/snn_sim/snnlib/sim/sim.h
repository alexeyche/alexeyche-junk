#pragma once


#include <snnlib/layers/layer.h>

class Sim: public Printable {
public: 
    Sim() {
        layers.push_back( unique_ptr<LayerObj>(new IaFLayer<IaFNeuron>(0, 100)) );
    }
    
    void print(std::ostream& str) const {
        for(auto it=layers.begin(); it!=layers.end(); ++it) {
            str << **it;
        }
    }
    
    vector< unique_ptr<LayerObj> > layers;
};


