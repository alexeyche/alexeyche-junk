#pragma once

#include "neuron.h"

#include <snnlib/layers/act_funcs/act_func.h>
#include <snnlib/learning/learning_rule.h>

class LayerObj : public Entity {
public:    
    LayerObj() {}
    void init(size_t _id, size_t _size, ConstObj &_c, ActFunc &_act, LearningRule &_lrule) {
        id = _id;
        N = _size;
        act = shared_ptr<const ActFunc>(&_act);
        lrule = shared_ptr<const LearningRule>(&_lrule);
        c = shared_ptr<const ConstObj>(&_c);
    }
    size_t id;
    size_t N;
    
    shared_ptr<const ActFunc> act;
    shared_ptr<const LearningRule> lrule;
    shared_ptr<const ConstObj> c;
};

template <typename T>
class Layer: public LayerObj {
public:
    Layer() {}
    void init(size_t _id, size_t _size, ConstObj &_c, ActFunc &_act, LearningRule &_lrule) {
        LayerObj::init(_id, _size, _c, _act, _lrule) {
        for(size_t i=0; i<N; i++) {
            neurons.push_back( unique_ptr<T>(new T(global_neuron_index++)) );
        }
    }
    

    void print(std::ostream& str) const {
        for(auto it=neurons.begin(); it!=neurons.end(); ++it) {
            str << **it;
        }
    }

    
    vector< unique_ptr<T> > neurons;
};


template <typename T>
class SRMLayer : public Layer<T> {
public:    
    SRMLayer() {}
};

