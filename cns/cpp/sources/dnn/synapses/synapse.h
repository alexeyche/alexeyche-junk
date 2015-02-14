#pragma once


#include <dnn/neurons/neuron.h>

namespace dnn {


template <typename Constants, typename State>
class Synapse : public ISynapse {
public:
	Synapse(Constants _c) : c(_c) {}
protected:
	Constants c;
	State s;	
};


}