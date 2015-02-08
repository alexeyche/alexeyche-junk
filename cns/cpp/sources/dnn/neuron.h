#pragma once

#include "base.h"

namespace dnn {

template <typename State, typename Eval>
class NeuronInput : public DynamicObject<State, Eval> {
public:

};


template <typename State, typename Eval = double, typename InputEval = double, typename SynapseSystemEval = double>
class SpikeNeuron : public DynamicObject<State, Eval> {
public:
    typedef EvalObject<InputEval> InputType;
    typedef EvalObject<SynapseSystemEval> SynapsesType;

    SpikeNeuron(InputType &i, SynapsesType &syn) : input(i), synapses(syn) {}
    
    virtual void spiked() = 0;

    InputType &input;
    SynapsesType &synapses;
};

template <typename InputEval>
class SpikeActFunction : public EvalObject<bool> {
public:
    typedef EvalObject<InputEval> NeuronType;

    SpikeActFunction(NeuronType &_n) : neuron(_n) {}
    virtual bool eval_spike() = 0;
    
    bool eval() {
    	bool spiked = eval_spike();
    	if(spiked) neuron.eval_spike();
    	return spiked;
    }

    NeuronType &neuron;
};

class NullEvalObject : public EvalObject<double> {
public:
    double eval() { return 0.0; }
};



}
