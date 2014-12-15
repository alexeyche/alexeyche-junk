#pragma once


#include "weight_normalization.h"

class SoftMinMax : public WeightNormalization {
protected:
    SoftMinMax() {
        Serializable::init(ESoftMinMax);
    }
    friend class Factory;
public:
    SoftMinMax(const SoftMinMaxC *_c, Neuron *_n) {
        init(_c, _n);
    }
    void init(const ConstObj *_c, Neuron *_n) {
        c = castType<SoftMinMaxC>(_c);
        n = _n;
        Serializable::init(ESoftMinMax);
    }
    void preModifyMeasure() {}

    double ltpMod(const double &w) {
        return c->nu_plus * (c->w_max - w);
    }

    double ltdMod(const double &w) {
        return c->nu_minus * w;
    }

    void modifyWeightDerivative(double &dw, const size_t &syn_id) {
        dw += ltpMod(n->syns[syn_id]->w) - ltdMod(n->syns[syn_id]->w);
    }

    void provideRuntime(WeightNormalizationRuntime &rt, Mode m) {
        if(m == WeightNormalization::LtpLtd) {
            rt.ltpMod = MakeDelegate(this, &WeightNormalization::ltpMod);
            rt.ltdMod = MakeDelegate(this, &WeightNormalization::ltdMod);
            rt.modifyWeightDerivative = &WeightNormalization::modifyWeightDerivativeDefault;
        } else {
            rt.ltpMod = &WeightNormalization::ltpModDefault;
            rt.ltdMod = &WeightNormalization::ltdModDefault;
            rt.modifyWeightDerivative = MakeDelegate(this, &SoftMinMax::modifyWeightDerivative);
        }
        rt.preModifyMeasure = MakeDelegate(this, &SoftMinMax::preModifyMeasure);
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

    const SoftMinMaxC *c;
};
