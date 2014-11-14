#pragma once

#include "layer.h"

template <typename NEURON>
class SRMLayer : public Layer<NEURON> {
private:    
    SRMLayer() {}
    friend class Factory;
public:
    void init(size_t _id, size_t _size, const ConstObj *_c, const ActFunc *_act, const LearningRule *_lrule) {
        Layer<NEURON>::init(_id, _size, _c, _act, _lrule);
        CAST_TYPE(LayerObj, SRMLayer, SRMLayerC)
        c = shared_ptr<const SRMLayerC>(cast);
    }

    
    shared_ptr<const SRMLayerC> c;
};


