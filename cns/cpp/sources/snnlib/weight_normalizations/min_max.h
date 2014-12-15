#pragma once


#include "weight_normalization.h"

class MinMax : public WeightNormalization {
protected:
    MinMax() {
        Serializable::init(EMinMax);
    }
    friend class Factory;
public:
    MinMax(const MinMaxC *_c, Neuron *_n) {
        init(_c, _n);
    }
    void init(const ConstObj *_c, Neuron *_n) {
        c = castType<MinMaxC>(_c);
        n = _n;
        Serializable::init(EMinMax);
    }
    void preModifyMeasure() {}

    void modifyWeightDerivative(double &dw, const size_t &syn_id) { 
        if((n->syns[syn_id]->w >= c->w_max)||(n->syns[syn_id]->w <= c->w_min)) dw = 0.0;
    }
	void deserialize() {
    }
    ProtoPack serialize() {
        return getNew();
    }
    ProtoPack getNew() {
        return ProtoPack();
    }

    void print(std::ostream& str) const { }

    double p_acc;

    const MinMaxC *c;
};