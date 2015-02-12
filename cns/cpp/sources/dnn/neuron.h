#pragma once

#include "base.h"

namespace dnn {

template <typename State, typename Eval>
using NeuronInput = DynamicObject<State, Eval>;


template <typename State,
		  typename Eval = bool,
		  typename InputEval = double,
		  typename SynapseSystemEval = double,
		  typename ActFunctionEval = bool>
class SpikeNeuron : public DynamicObject<State, Eval> {
public:
    typedef EvalObject<InputEval> InputType;
    typedef EvalObject<SynapseSystemEval> SynapsesType;
    typedef EvalObject<ActFunctionEval> ActFunctionType;

    SpikeNeuron(InputType &i, SynapsesType &syn, ActFunctionType &af) : input(i), synapses(syn), act_f(af) {}

    ActFunctionType &act_f;
    InputType &input;
    SynapsesType &synapses;
};

template <typename InputEval>
class SpikeActFunction : public EvalObject<bool> {
public:
    typedef EvalObject<InputEval> NeuronType;

    SpikeActFunction(NeuronType &_n) : neuron(_n) {}


    NeuronType &neuron;
};

class NullEvalObject : public EvalObject<double> {
public:
    double eval() { return 0.0; }
};



}
