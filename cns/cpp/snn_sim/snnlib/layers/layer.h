#pragma once

#include "neuron.h"

class LayerObj : public Entity {
};

template <typename T>
class Layer: public LayerObj {
public:
    Layer() {}
    Layer(size_t _id, size_t size) : id(_id), N(size) {
        for(size_t i=0; i<N; i++) {
            neurons.push_back( unique_ptr<T>(new T(global_neuron_index++)) );
        }
    }

    size_t id;
    size_t N;

    void print(std::ostream& str) const {
        for(auto it=neurons.begin(); it!=neurons.end(); ++it) {
            str << **it;
        }
    }
    vector< unique_ptr<T> > neurons;
};


template <typename T>
class IaFLayer : public Layer<T> {
public:
    IaFLayer() {}
    IaFLayer(size_t _id, size_t size) : Layer<T>(_id, size) {
    }
    
};

