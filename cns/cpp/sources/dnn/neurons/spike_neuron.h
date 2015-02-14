#pragma once


#include "neuron.h"

namespace dnn {


class ISpikeNeuron {
public:
	virtual double& getMembrane() = 0;
	virtual double& getSpikeProbability() = 0;
	virtual vector<ISynapse*>& getSynapses() = 0;
};

template <typename Constants, typename State>
class SpikeNeuron : public Neuron<Constants, State, void, bool>, public ISpikeNeuron {
public:
	typedef Neuron<Constants, State, void, bool> Parent;
	SpikeNeuron(Constants _c) : Parent(_c) {}

	vector<ISynapse*>& getSynapses() {
		return Parent::synapses;
	}
};


}