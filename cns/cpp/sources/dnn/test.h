#pragma once

#include "base.h"
#include "neuron.h"
#include "synapse.h"

namespace dnn {

class InputTimeSeriesState : public BaseState<1> {};

class InputTimeSeries : public NeuronInput<InputTimeSeriesState, double> {
public:
    double eval() {
        return 5;
    }
    void step(InputTimeSeriesState &dState_dt) {

    }
};


class LeakyIntegrateAndFireState : public BaseState<1> {
};

class LeakyIntegrateAndFire : public Neuron<LeakyIntegrateAndFireState> {
public:
    LeakyIntegrateAndFire(InputType &inp, SynapsesType &syn) : Neuron(inp, syn) {}
    double eval() {
        return 2*input.eval() + synapses.eval();
    }
    void step(LeakyIntegrateAndFireState &dState_dt) {

    }
};


class Determ : public ActFunction<double, bool> {
public:
    Determ(NeuronType &_n) : ActFunction(_n) {}
    bool eval() {
        double e = neuron.eval();
        if(e>5) return true;
        return false;
    }
};


}

void test();
