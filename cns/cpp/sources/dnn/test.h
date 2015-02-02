#pragma once

#include <iostream>
#include <memory>
#include <vector>

using std::cout;
using std::move;
using std::unique_ptr;
using std::vector;

void test();



template <typename Constants, typename State>
class DynamicObject {
public:
    DynamicObject(const Constants &_c) : c(_c) {}
	virtual void step(State &dState_dt) = 0;
	

	State state;
protected:
    const Constants &c;
};

class NeuronState : public vector<double> {
		
};

struct NeuronConstants {
	double tau_leak;
};

class Neuron : public DynamicObject<NeuronConstants, State> {
public:
	Neuron(NeuronConstants &c) : DynamicObject(c) {}
	
	void step(State &dState_dt) {
		dState_dt[0] = state[0]/c.tau_leak; 
	}
	
};

