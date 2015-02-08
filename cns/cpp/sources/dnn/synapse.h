#pragma once

#include "base.h"

namespace dnn {


template <typename State, typename Eval>
class Synapse : public DynamicObject<State, Eval> {

};

template <typename Elem, typename Eval>
class SynapseSystem : public System<Elem, Eval> {
	double eval() {
		double sum = 0.0;
		for(auto it=System<Elem,Eval>::sys.begin(); it != System<Elem,Eval>::sys.end(); ++it) { 
			sum += (*it)->eval();
		}
		return sum;
	}
};


class StaticSynapseState : public BaseState<1> {

};

class StaticSynapse : public Synapse<StaticSynapseState, double> {
public:
    double eval() {
        return 2;
    }
    
    void step(StaticSynapseState &dState_dt) {

    }	
};

typedef SynapseSystem<StaticSynapse, double> StaticSynapseSystem;



}
