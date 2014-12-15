#pragma once

#include <snnlib/serialize/serialize.h>
#include <snnlib/protos/model.pb.h>


typedef FastDelegate2<double &,const size_t &> derivModDelegate;


struct WeightNormalizationRuntime {
    funDelegate ltpMod;
    funDelegate ltdMod;
    derivModDelegate modifyWeightDerivative;
    stateDelegate preModifyMeasure;
};


class WeightNormalization : public Serializable<Protos::BlankModel> {
public:
    enum Mode {LtpLtd, Derivative};

    WeightNormalization() : Serializable(EBlankModel) {}
	virtual void init(const ConstObj *_c, Neuron *_n) = 0;
	virtual void preModifyMeasure() = 0;
    virtual void modifyWeightDerivative(double &dw, const size_t &syn_id) = 0;
    virtual double ltpMod(const double &w) { return 1.0; }
    virtual double ltdMod(const double &w) { return 1.0; }
    virtual void provideRuntime(WeightNormalizationRuntime &rt, Mode m) = 0;

    //default runtime:
    static void preModifyMeasureDefault() {}
    static void modifyWeightDerivativeDefault(double &dw, const size_t &syn_id) {}
    static double ltpModDefault(const double &w) { return 1.0; }
    static double ltdModDefault(const double &w) { return 1.0; }
    static void provideDefaultRuntime(WeightNormalizationRuntime &rt) {
        rt.ltpMod = &WeightNormalization::ltpModDefault;
        rt.ltdMod = &WeightNormalization::ltdModDefault;
        rt.preModifyMeasure = &WeightNormalization::preModifyMeasureDefault;
        rt.modifyWeightDerivative = &WeightNormalization::modifyWeightDerivativeDefault;
    }

protected:
    Neuron *n;
    Mode m;
};

