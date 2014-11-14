#pragma once


class Factory;


#include "layer.h"


template <typename T>
class SigmaTCLayer: public Layer<T> {
protected:
    SigmaTCLayer() {}
    friend class Factory;
public:    
    void init(size_t _id, size_t _size, const ConstObj *_c, const ActFunc *_act, const LearningRule *_lrule) {
        if(_lrule) {
            cerr << "Error: found learning rule in input layer.\n";
            terminate();
        }
        Layer<T>::init(_id, _size, _c, _act, _lrule);
        CAST_TYPE(LayerObj, SigmaTCLayer, SigmaTCLayerC)
        c = shared_ptr<const SigmaTCLayerC>(cast);
    }


private:
    shared_ptr<const SigmaTCLayerC> c;

};

