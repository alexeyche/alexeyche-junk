#pragma once

#include "core.h"
#include "active_container.h"

namespace dnn {





//DynamicObject returns State
//System returns States 

// Input ->|
//         |DynamicObject-> FunEventGenerator
// System->|
//
// or
// 
// Fun( DynamicObject(Fun(Input), System) )

// Fun1< DynamicObject0<Input, Fun0<System0>> > sys = funEval(Do2InputEval(inputEval(), funEval(System)));
// sys.eval();
// 

template <typename T>
class Object {
    virtual T& eval() { return s; }
protected:
    T &s;
};


class Fun {
public:    
    virtual State operator() (State& o) = 0;
};




template <typename StateType>
class DynamicObjectBase {
public:
    typedef vector<StateType> State;
    virtual void step(State &dState_dt) = 0;
    virtual void init() = 0;
    virtual const StateType& output() { return state[0]; }
protected:
    State state;
};


typedef double DefaultStateType;

template <typename Constants>
class DynamicObject : public DynamicObjectBase<DefaultStateType> {
public:
    DynamicObject(const Constants &_c) : c(_c) {}
protected:
    const Constants &c;

};




struct SynapseConstants {
    double tau_syn;
};

class Synapse : public DynamicObject<SynapseConstants> {
    Synapse(const SynapseConstants &c) : DynamicObject(c) {}

    void init() {
        state[0] = 0.0;
    }
    void step(State &dState_dt) {
        dState_dt[0] = state[0]/c.tau_syn;
    }
};


struct NeuronConstants {
    double tau_leak;
};

class Neuron : public DynamicObject<NeuronConstants> {
public:
    Neuron(const NeuronConstants &c) : DynamicObject(c) {}

    void init() {
        state[0] = 0.0;
    }
    void step(State &dState_dt) {
        dState_dt[0] = state[0]/c.tau_leak;
    }

};


}
