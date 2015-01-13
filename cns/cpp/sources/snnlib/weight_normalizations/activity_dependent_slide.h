#pragma once


#include "weight_normalization.h"

class ActivityDependentSlide : public WeightNormalization {
protected:
    ActivityDependentSlide() {
        Serializable::init(EActivityDependentSlide);
    }
    friend class Factory;
public:
    ActivityDependentSlide(const ActivityDependentSlideC *_c, Neuron *_n) {
        init(_c, _n);
    }
    void init(const ConstObj *_c, Neuron *_n) {
        c = castType<ActivityDependentSlideC>(_c);
        n = _n;
        p_acc = 0.0;
        Serializable::init(EActivityDependentSlide);
    }
    void preModifyMeasure() {
        p_acc += (-p_acc+n->fired)/c->tau_mean;
        __p_acc_cube = p_acc * p_acc * p_acc;
	}

    void modifyWeightDerivative(double &dw, const size_t &syn_id) {
        if(n->glob_c->getSimTime() < c->tau_mean) dw = 0.0;
    }
    double ltdMod(const double &w) {
        return __p_acc_cube / c->__target_rate_cube;
    }
    void provideRuntime(WeightNormalizationRuntime &rt, Mode m) {
        rt.ltpMod = &WeightNormalization::ltpModDefault;
        rt.ltdMod = MakeDelegate(this, &ActivityDependentSlide::ltdMod);
        rt.preModifyMeasure = MakeDelegate(this, &ActivityDependentSlide::preModifyMeasure);
        rt.modifyWeightDerivative = MakeDelegate(this, &ActivityDependentSlide::modifyWeightDerivative);
    }

    void deserialize() {
        Protos::ActivityDependentSlide *mess = getSerializedMessage<Protos::ActivityDependentSlide>();
        p_acc = mess->p_acc();
    }
    ProtoPack serialize() {
        Protos::ActivityDependentSlide *mess = getNewMessage<Protos::ActivityDependentSlide>();
        mess->set_p_acc(p_acc);
        return ProtoPack({mess});
    }
    ProtoPack getNew() {
        return ProtoPack({ getNewMessage<Protos::ActivityDependentSlide>() });
    }

    void print(std::ostream& str) const { }

    double p_acc;
    double __p_acc_cube;
    const ActivityDependentSlideC *c;
};
