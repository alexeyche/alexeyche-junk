#pragma once

#include "base.h"

struct InputTimeSeriesState : public State<1> {};

class InputTimeSeries : public NeuronInput<InputTimeSeriesState, double> {
public:
    double eval() {
        return 5;
    }    
    void step(InputTimeSeriesState &dState_dt) {

    }
};


struct LeakyIntegrateAndFireState {
};

class LeakyIntegrateAndFire : public Neuron< Object<double>&, LeakyIntegrateAndFireState, double> {
public:    
    LeakyIntegrateAndFire(Object<double> &o) : Neuron<Object<double>&, LeakyIntegrateAndFireState, double>(o) {}
    double eval() {
        return 2*o.eval();
    }
    void step(LeakyIntegrateAndFireState &dState_dt) {

    }
};


void test();
