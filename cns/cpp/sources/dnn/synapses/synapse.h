#pragma once


#include <dnn/neurons/neuron.h>

namespace dnn {


template <typename Constants, typename State>
class Synapse : public ISynapse, public StateObject<Constants, State>  {
public:
	Synapse() {}
};


}
