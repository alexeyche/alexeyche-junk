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

    double ltpMod(const double &w) {
        double delta = c->w_max - w;
        if(delta<=0) return 0.0;
        return delta * c->nu_plus;
    }

    double ltdMod(const double &w) {
        if(w<=0) return 0.0;
        return w * c->nu_minus;
    }

    void modifyWeightDerivative(double &dw, const size_t &syn_id) {
        if((n->syns[syn_id]->w >= c->w_max)||(n->syns[syn_id]->w <= 0.0)) dw = 0.0;
    }

    void provideRuntime(WeightNormalizationRuntime &rt, Mode m) {
        if(m == WeightNormalization::LtpLtd) {
            rt.ltpMod = MakeDelegate(this, &WeightNormalization::ltpMod);
            rt.ltdMod = MakeDelegate(this, &WeightNormalization::ltdMod);
            rt.modifyWeightDerivative = &WeightNormalization::modifyWeightDerivativeDefault;
        } else {
            rt.ltpMod = &WeightNormalization::ltpModDefault;
            rt.ltdMod = &WeightNormalization::ltdModDefault;
            rt.modifyWeightDerivative = MakeDelegate(this, &MinMax::modifyWeightDerivative);
        }
        rt.preModifyMeasure = MakeDelegate(this, &MinMax::preModifyMeasure);
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

    const MinMaxC *c;
};
