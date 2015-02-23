#pragma once

#include "learning_rule.h"
#include <dnn/protos/generated.pb.h>

namespace dnn {


/*@GENERATE_PROTO@*/
struct StdpConstants : public Serializable<Protos::StdpConstants>  {
    double a_plus;

    void processStream(Stream &str) {
        acquire(str) << "a_plus: " << a_plus << Self::End;
    }
};

/*@GENERATE_PROTO@*/
struct StdpState : public Serializable<Protos::StdpState> {
    double x;

    void processStream(Stream &str) {
        acquire(str) << "x: " << x << Self::End;
    }
};

class Stdp : public LearningRule<StdpConstants, StdpState> {
public:
	typedef LearningRule<StdpConstants, StdpState> Parent;

	Stdp() {}

	void evaluate() {
		vector<ISynapse*> syns = neuron->getSynapses();
	}

};



}
