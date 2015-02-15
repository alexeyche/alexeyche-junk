#pragma once

#include "spike_neuron.h"
#include <dnn/io/stream.h>
#include <dnn/protos/generated.pb.h>
#include <dnn/io/serialize.h>

namespace dnn {

struct IAFConstants {};

/*@GENERATE_PROTO@*/
struct IAFState : public Serializable<Protos::IAFState> {
	double p;
	double u;

	void out(OStream &str) const {
		str << name() << "u: " << u << "p: " << p << OStream::end;
	}
	void in(IStream &in) const {

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