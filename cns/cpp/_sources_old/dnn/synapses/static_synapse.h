#pragma once

#include "synapse.h"
#include <dnn/protos/generated.pb.h>

namespace dnn {

/*@GENERATE_PROTO@*/
struct StaticSynapseConstants : public Serializable<Protos::StaticSynapseConstants> {
    double epsp_decay;

    void processStream(Stream &str) {
        acquire(str) << "epsp_decay: " << epsp_decay << Self::End;
    }
};

/*@GENERATE_PROTO@*/
struct StaticSynapseState  : public Serializable<Protos::StaticSynapseState> {
	double x;
	double w;

    void processStream(Stream &str) {
        acquire(str) << "x: " << x << "w: " << w << Self::End;
    }
};

class StaticSynapse : public Synapse<StaticSynapseConstants, StaticSynapseState> {
public:
	typedef Synapse<StaticSynapseConstants, StaticSynapseState> Parent;
	StaticSynapse() {}

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
