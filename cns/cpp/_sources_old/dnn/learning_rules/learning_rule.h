#pragma once

#include <dnn/neurons/neuron.h>

namespace dnn {

template <typename Constants, typename State>
class LearningRule : public DynamicObject<void>, public StateObject<Constants, State> {
public:
	LearningRule() : neuron(nullptr) {}
    void setNeuron(ISpikeNeuron &_neuron) {
        neuron = _neuron;
    }
protected:
	ISpikeNeuron *neuron;

	Constants c;
	State s;
};

}
