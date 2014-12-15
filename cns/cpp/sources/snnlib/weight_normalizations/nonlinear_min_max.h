#pragma once


#include "weight_normalization.h"

#include <snnlib/util/fastapprox/fastpow.h>

class NonlinearMinMax : public WeightNormalization {
protected:
    NonlinearMinMax() {
        Serializable::init(ENonlinearMinMax);
    }
    friend class Factory;
public:
    NonlinearMinMax(const NonlinearMinMaxC *_c, Neuron *_n) {
        init(_c, _n);
    }
    void init(const ConstObj *_c, Neuron *_n) {
        c = castType<NonlinearMinMaxC>(_c);
        n = _n;
        Serializable::init(ENonlinearMinMax);
    }
    void preModifyMeasure() {}

    double ltpMod(const double &w) {
        return fasterpow( 1 - w/c->w_max, c->mu );
    }

    double ltdMod(const double &w) {
        return c->depression_factor * fastpow( w/c->w_max, c->mu );
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
            rt.modifyWeightDerivative = MakeDelegate(this, &NonlinearMinMax::modifyWeightDerivative);
        }
        rt.preModifyMeasure = MakeDelegate(this, &NonlinearMinMax::preModifyMeasure);
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

    const NonlinearMinMaxC *c;
};
