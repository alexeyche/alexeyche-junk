#pragma once

#include <dnn/neurons/neuron.h>

namespace dnn {

template <typename Constants, typename State>
class LearningRule : public DynamicObject<void> {
public:
	LearningRule(Constants _c, ISpikeNeuron &_neuron) : c(_c), neuron(_neuron) {}

protected:
	ISpikeNeuron &neuron;

	Constants c;
	State s;	
};

}