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

class LeakyIntegrateAndFire : public SpikeNeuron<LeakyIntegrateAndFireState> {
public:
    LeakyIntegrateAndFire(InputType &inp, SynapsesType &syn) : SpikeNeuron(inp, syn) {}
    double eval() {
        return 2*input.eval() + synapses.eval();
    }

    void spiked() {

    }
    void step(LeakyIntegrateAndFireState &dState_dt) {

    }
};


class Determ : public SpikeActFunction<double> {
public:
    Determ(NeuronType &_n) : SpikeActFunction(_n) {}
    bool eval_spike() {
    	double e = neuron.eval();
        if(e>5) return true;
        return false;
    }
    
};


}

void test();
