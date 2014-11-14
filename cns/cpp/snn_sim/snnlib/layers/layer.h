#pragma once

#include "neuron.h"

#include <snnlib/layers/act_funcs/act_func.h>
#include <snnlib/learning/learning_rule.h>
#include <snnlib/config/constants.h>

class Factory;


class LayerObj : public Entity {
protected:
    friend class Factory;
    LayerObj() {}
public:    
    LayerObj(size_t _id, size_t _size, const ConstObj *_c, const ActFunc *_act, const LearningRule *_lrule) {
        init(_id,_size,_c,_act,_lrule);
    }
    virtual void init(size_t _id, size_t _size, const ConstObj *_c, const ActFunc *_act, const LearningRule *_lrule) {
        id = _id;
        N = _size;
        act = shared_ptr<const ActFunc>(_act);
        lrule = shared_ptr<const LearningRule>(_lrule);
        c = shared_ptr<const ConstObj>(_c);
    }
    size_t id;
    size_t N;
    
    shared_ptr<const ActFunc> act;
    shared_ptr<const LearningRule> lrule;
    shared_ptr<const ConstObj> c;
};

template <typename T>
class Layer: public LayerObj {
protected:
    Layer() {}
    friend class Factory;
public:
    Layer(size_t _id, size_t _size, const ConstObj *_c, const ActFunc *_act, const LearningRule *_lrule) {
        init(_id,_size,_c,_act,_lrule);
    }
    void init(size_t _id, size_t _size, const ConstObj *_c, const ActFunc *_act, const LearningRule *_lrule) {
        LayerObj::init(_id, _size, _c, _act, _lrule);
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


template <typename N>
class SRMLayer : public Layer<N> {
private:    
    SRMLayer() {}
    friend class Factory;
public:
    void init(size_t _id, size_t _size, const ConstObj *_c, const ActFunc *_act, const LearningRule *_lrule) {
        Layer<N>::init(_id, _size, _c, _act, _lrule);
    }

};


