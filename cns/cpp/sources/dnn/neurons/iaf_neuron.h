#pragma once

#include "spike_neuron.h"
#include <dnn/io/stream.h>

namespace dnn {

struct IAFConstants {};


struct IAFState {
	double p;
	double u;

	void out(OStream &str) const {
		str << "u: " << u << "p: " << p << OStream::end;
	}
};

class IAFNeuron : public SpikeNeuron<IAFConstants, IAFState> {
public:
	typedef SpikeNeuron<IAFConstants, IAFState> Parent;
	IAFNeuron(IAFConstants _c) : Parent(_c) {}

	void evaluate() {

	}

	double& getMembrane() {
		return s.u;
	}
	double& getSpikeProbability() {
		return s.p;
	}
};



}