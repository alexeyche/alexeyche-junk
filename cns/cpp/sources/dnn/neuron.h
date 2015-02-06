#pragma once

#include "base.h"

namespace dnn {


template <typename State, typename Eval = double, typename InputEval = double, typename SynapseSystemEval = double>
class Neuron : public DynamicObject<State, Eval> {
public:
    typedef EvalObject<InputEval> InputType;
    typedef EvalObject<SynapseSystemEval> SynapsesType;

    Neuron(InputType &i, SynapsesType &syn) : input(i), synapses(syn) {}

    InputType &input;
    SynapsesType &synapses;
};

template <typename InputEval, typename OutputEval>
class ActFunction : public EvalObject<OutputEval> {
public:
    typedef EvalObject<InputEval> NeuronType;

    ActFunction(NeuronType &_n) : neuron(_n) {}

    NeuronType &neuron;
};


template <typename OutputEval>
class NeuronScheme : public ComplexObject<OutputEval> {
public:

};

}
