#pragma once


class Factory;


#include "tuning_curve.h"

#include <snnlib/neurons/neuron.h>

class SigmaTuningCurve: public TuningCurve {
protected:
    SigmaTuningCurve() {}
    friend class Factory;
public:
    void init(const ConstObj *_c, size_t layer_size, size_t neuron_id, Neuron *_n) {
        c = castType<SigmaTuningCurveC>(_c);
        n = _n;
        center = c->intercept.first + ((double)neuron_id/layer_size)*(c->intercept.second-c->intercept.first);
        gain = getUnifBetween(c->gain.first, c->gain.second);
        sigma = getUnifBetween(c->sigma.first, c->sigma.second);
        sigma_square = sigma*sigma;
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

