#pragma once

#include "synapse.h"

namespace dnn {


struct StaticSynapseConstants {};

struct StaticSynapseState {
	double x;
	double w;
};

class StaticSynapse : public Synapse<StaticSynapseConstants, StaticSynapseState> {
public:
	typedef Synapse<StaticSynapseConstants, StaticSynapseState> Parent;
	StaticSynapse(StaticSynapseConstants _c) : Parent(_c) {}

	double evaluate() {

	}
	double& getMembrane() {
		return s.x;
	}
	void modifyWeight(const double &dw) {
		s.w += dw;
	}
};


}