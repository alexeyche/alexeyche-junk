#pragma once

#include "learning_rule.h"

namespace dnn {



struct StdpConstants  {};

struct StdpState {};

class Stdp : public LearningRule<StdpConstants, StdpState> {
public:
	typedef LearningRule<StdpConstants, StdpState> Parent;

	Stdp(StdpConstants _c, ISpikeNeuron &_neuron) : Parent(_c, _neuron) {}

	void evaluate() {
		vector<ISynapse*> syns = neuron.getSynapses();
	}

};



}