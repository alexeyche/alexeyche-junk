#pragma once

#include <snnlib/core.h>

class SimNeuron {
protected:
	SimNeuron()  {}
public:
	virtual void calculateProbability() = 0;
	virtual void calculateDynamics() = 0;
protected:
	list<Synapse*> active_synapses;
};