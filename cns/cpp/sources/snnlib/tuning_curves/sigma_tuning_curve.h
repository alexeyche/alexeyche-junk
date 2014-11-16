#pragma once


class Factory;


#include "tuning_curve.h"

#include <snnlib/layers/neuron.h>

class SigmaTuningCurve: public TuningCurve {
protected:
    SigmaTuningCurve() {}
    friend class Factory;
public:    
    void init(size_t _size, const ConstObj *_c) {
        CAST_TYPE(SigmaTuningCurveC, _c)
        c = shared_ptr<const SigmaTuningCurveC>(cast);

        centers = new double[_size];
        gains = new double[_size];
        sigma = new double[_size];
        for(size_t ni=0; ni<_size; ni++) {
            centers[ni] = getUnifBetween(c->intercept.first, c->intercept.second);
            gains[ni] = getUnifBetween(c->gain.first, c->gain.second);
            sigma[ni] = getUnifBetween(c->sigma.first, c->sigma.second);
        } 
    }

    ~SigmaTuningCurve() {
        delete []centers;
        delete []gains;
        delete []sigma;
    }
    
    void calculateResponse(Neuron *n, const double &x) {
//        n->y = gains[ni]*exp( - (x - centers[ni])*(x - centers[ni]) / ( 2 * sigma[ni]*sigma[ni] ));
    }

    

private:
    double *centers;
    double *gains;
    double *sigma;
    shared_ptr<const SigmaTuningCurveC> c;

};

