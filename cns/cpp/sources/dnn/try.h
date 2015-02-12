#pragma once

#include "core.h"

namespace dnn {

template <typename Eval>
class EvalObject {
public:
    typedef Eval eval_type;
    virtual Eval eval() = 0;
};



template<typename State, typename Eval>
class DynamicObject : public EvalObject<Eval> {
public:
    typedef unique_ptr<State> StatePtr;

    DynamicObject() : state(new State) {}

protected:
    unique_ptr<State> state;
};




struct NeuronInputState {

};



class Input : public DynamicObject<NeuronInputState, double> {
public:
    typedef unique_ptr<Input> ptr;
    Input() {}
    double eval() {}

};


struct SynapseState {

};


class Synapse : public DynamicObject<SynapseState, double> {
public:
};


struct NeuronState {
    bool fired;
};

class Neuron: public DynamicObject<NeuronState, double> {
public:
    Neuron(unique_ptr<Input> _input) : input(std::move(_input)) {}
    double eval() {}
protected:
    unique_ptr<Input> input;
};





}



