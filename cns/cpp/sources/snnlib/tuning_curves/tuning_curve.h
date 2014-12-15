#pragma once

#include <snnlib/base.h>


struct TuningCurveRuntime {
    funDelegate calculateResponse;
};

class TuningCurve : public Obj {
public:
	virtual void init(const ConstObj *_c, size_t layer_size, size_t neuron_id, Neuron *_n) = 0;
	virtual double calculateResponse(const double &I) = 0;
    virtual void provideRuntime(TuningCurveRuntime &rt) = 0;

    static double calculateDefaultResponse(const double &I) {
        return 0.0;
    }
    static void provideDefaultRuntime(TuningCurveRuntime &rt) {
        rt.calculateResponse = &TuningCurve::calculateDefaultResponse;
    }

};


