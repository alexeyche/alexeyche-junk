#pragma once

#include "core.h"
#include "active_container.h"

namespace dnn {

template <typename StateType>
class DynamicObjectBase {
public:
    typedef vector<StateType> State;
    virtual void step(State &dState_dt) = 0;
    virtual void init() = 0;
    virtual const StateType& output() { return state[0]; }
protected:
    State state;
    ActiveContainer<StateType> inputs;
};
class DynamicObjectBaseDeleter {
public:
    void operator() (DynamicObjectBase *o) {
        delete o;
    }
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
