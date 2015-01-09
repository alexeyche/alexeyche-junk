#pragma once


#include "weight_normalization.h"

class MeanActivityHomeostasis : public WeightNormalization {
protected:
    MeanActivityHomeostasis() {
        Serializable::init(EMeanActivityHomeostasis);
    }
    friend class Factory;
public:
    MeanActivityHomeostasis(const MeanActivityHomeostasisC *_c, Neuron *_n) {
        init(_c, _n);
    }
    void init(const ConstObj *_c, Neuron *_n) {
        c = castType<MeanActivityHomeostasisC>(_c);
        n = _n;
        Serializable::init(EMeanActivityHomeostasis);
    }
    void preModifyMeasure() {
		if(n->fired) {
			p_acc += 1.0;
		}
		p_acc += -p_acc/c->tau_mean;
	}

    void modifyWeightDerivative(double &dw, const size_t &syn_id) {

    }
    void provideRuntime(WeightNormalizationRuntime &rt, Mode m) {
        rt.ltpMod = &WeightNormalization::ltpModDefault;
        rt.ltdMod = &WeightNormalization::ltdModDefault;
        rt.preModifyMeasure = MakeDelegate(this, &MeanActivityHomeostasis::preModifyMeasure);
        rt.modifyWeightDerivative = MakeDelegate(this, &MeanActivityHomeostasis::modifyWeightDerivative);
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

    const MeanActivityHomeostasisC *c;
};
