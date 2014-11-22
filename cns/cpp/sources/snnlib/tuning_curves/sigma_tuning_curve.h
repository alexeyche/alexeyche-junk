#pragma once


class Factory;


#include "tuning_curve.h"

#include <snnlib/layers/neuron.h>

static size_t sigma_id = 0;

class SigmaTuningCurve: public TuningCurve {
protected:
    SigmaTuningCurve() {}
    friend class Factory;
public:
    void init(const ConstObj *_c, Neuron *_n) {
        CAST_TYPE(SigmaTuningCurveC, _c)
        c = cast;
        n = _n;
        n->setTuningCurve(this);
        center = c->intercept.first + (sigma_id/100.0)*(c->intercept.second-c->intercept.first);
        gain = getUnifBetween(c->gain.first, c->gain.second);
        sigma = getUnifBetween(c->sigma.first, c->sigma.second);
        sigma_square = sigma*sigma;
        sigma_id++;
    }

    ~SigmaTuningCurve() {
    }

    void calculateResponse(const double &x) {
        n->y = gain*exp( - (x - center)*(x - center) / ( 2 * sigma_square ));
    }



private:
    Neuron *n;
    double center;
    double gain;
    double sigma;
    double sigma_square;
    const SigmaTuningCurveC *c;

};

