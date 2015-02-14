#pragma once

#include <dnn/base/dynamic_object.h>
#include <dnn/core.h>

namespace dnn {

class ISynapse : public DynamicObject<double> {
    virtual void modifyWeight(const double &dw) = 0;
    virtual double& getMembrane() = 0;
};


template <  typename Constants,         /*user defined*/
            typename State,             /*user defined*/
            typename Eval,
            typename ActFunctionEval
         >
class Neuron : public DynamicObject<Eval> {
public:
    typedef DynamicObject<double> InputType;
    typedef DynamicObject<void> LearningRuleType;
    typedef DynamicObject<ActFunctionEval> ActFunctionType;
    typedef ISynapse SynapseType;

    Neuron(Constants _c) : 
        c(_c), input(nullptr), act_func(nullptr), lrule(nullptr) {

    }
        
    void setInput(InputType *_input) { input = _input; }
    void setActFunction(ActFunctionType *_act_func) { act_func = _act_func; }
    void setLearningRule(LearningRuleType *_lrule) { lrule = _lrule; }
    void addSynapse(SynapseType *syn) {
        synapses.push_back(syn);
    }
    
    const State& getState() {
        return s;
    }
protected:
    InputType* input;
    ActFunctionType* act_func;
    LearningRuleType* lrule;
    vector<SynapseType*> synapses;

    Constants c;
    State s;
};


}