#pragma once

#include "core.h"

namespace dnn {

template <typename Eval=void>
class DynamicObject {
public:
    virtual Eval evaluate() = 0;
};


template <
            typename Constants,         /*user defined*/
            typename State,             /*user defined*/
            typename InputEval,         /*system*/
            typename ActFunctionEval,   /*system*/
            typename LearningRuleEval   /*system*/
         >
class Neuron : public DynamicObject {
public:
    typename DynamicObject<InputEval> InputType;
    typename DynamicObject<ActFunctionEval> ActFunctionType;
    typename DynamicObject<LearningRuleEval> LearningRuleType;

    Neuron(Constants c, InputType _input, ActFunctionType _act_func, LearningRuleType _lrule)


protected:
    InputType input;
    ActFunctionType act_func;
    LearningRuleType lrule;

    Constants c;
    State s;
};

template <typename Constants, typename State>
using SpikeNeuron = Neuron<Constants, State, double, bool, void>;

class IAFConstants {};
class IAFState {};

class IAFNeuron : public SpikeNeuron<IAFConstants, IAFState> {
    void    evaluate() {

    }
};



}



