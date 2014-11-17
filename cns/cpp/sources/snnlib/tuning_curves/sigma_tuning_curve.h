#pragma once


class Factory;


#include "tuning_curve.h"

#include <snnlib/layers/neuron.h>

class SigmaTuningCurve: public TuningCurve {
protected:
    SigmaTuningCurve() {}
    friend class Factory;
public:
    void init(const ConstObj *_c) {
        CAST_TYPE(SigmaTuningCurveC, _c)
        c = cast;

        center = getUnifBetween(c->intercept.first, c->intercept.second);
        gain = getUnifBetween(c->gain.first, c->gain.second);
        sigma = getUnifBetween(c->sigma.first, c->sigma.second);
        sigma_square = sigma*sigma;
    }

    ~SigmaTuningCurve() {
    }

    double calculateResponse(const double &x) {
        return gain*exp( - (x - center)*(x - center) / ( 2 * sigma_square ));
    }



private:
    double center;
    double gain;
    double sigma;
    double sigma_square;
    const SigmaTuningCurveC *c;

};

