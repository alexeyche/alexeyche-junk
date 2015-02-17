#pragma once

#include "spike_neuron.h"
#include <dnn/io/stream.h>
#include <dnn/protos/generated.pb.h>
#include <dnn/io/serialize.h>
#include <dnn/base/factory.h>

namespace dnn {

/*@GENERATE_PROTO@*/
struct IAFConstants : public Serializable<Protos::IAFConstants> {

	double R;
	double C;

	void processStream(Stream &str) {
		acquire(str) << "R: " << R << "C: " << C << Self::End;
	}
};

/*@GENERATE_PROTO@*/
struct IAFState : public Serializable<Protos::IAFState> {
	double p;
	double u;

	void processStream(Stream &str) {
		acquire(str) << "u: " << u << "p: " << p << Self::End;
	}
};

class IAFNeuron : public SpikeNeuron<IAFConstants, IAFState> {
public:
	IAFNeuron() {}

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
