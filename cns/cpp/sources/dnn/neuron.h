#pragma once


template <typename State, typename Eval>
class NeuronInput : public DynamicObject<State, Eval> {
};

template <typename Arg, typename State, typename Eval>
class Neuron : public DynamicObjectDep<Arg, State, Eval> {
public:
    Neuron(Arg &o) : DynamicObjectDep<Arg, State, Eval>(o) {}
};
