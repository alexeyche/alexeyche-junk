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
        if(c->max_sigma_num < 1) {
            cerr << "Need max_sigma_num be greater or equal to 1\n";
            terminate();
        }
        while(centers.size() < c->max_sigma_num) {
            if(centers.size() == 0){ // first sigma uniform
                centers.push_back(c->intercept.first + ((double)neuron_id/layer_size)*(c->intercept.second-c->intercept.first));
            } else {
                centers.push_back(getUnifBetween(c->intercept.first, c->intercept.second));
            }

            gains.push_back(getUnifBetween(c->gain.first, c->gain.second));
            double sigma = getUnifBetween(c->sigma.first, c->sigma.second);
            sigmas_squared.push_back(sigma*sigma);

            if(c->prob_next_sigma < getUnif()) break;
        }
    }

    ~SigmaTuningCurve() {
    }

    double calculateResponse(const double &x) {
        double resp = 0.0;
        for(size_t si=0; si < centers.size(); si++) {
            resp += gains[si] * exp( - (x - centers[si])*(x - centers[si]) / ( 2 * sigmas_squared[si] ));
        }
        return resp;
    }
    void provideRuntime(TuningCurveRuntime &rt) {
        rt.calculateResponse = MakeDelegate(this, &SigmaTuningCurve::calculateResponse);
    }



private:
    Neuron *n;
    vector<double> centers;
    vector<double> gains;
    vector<double> sigmas_squared;
    const SigmaTuningCurveC *c;

};

