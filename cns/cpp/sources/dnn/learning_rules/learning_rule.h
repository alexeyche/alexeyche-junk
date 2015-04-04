#pragma once

#include <dnn/io/serialize.h>
#include <dnn/util/statistics.h>
#include <dnn/neurons/spike_neuron.h>

namespace dnn {


struct LearningRuleInterface {
	propSynSpikeDelegate propagateSynapseSpike;
	calculateDynamicsDelegate calculateDynamics;
};


class Network;
class Builder;

class LearningRuleBase : public SerializableBase {
friend class Network;
friend class Builder;
public:
	typedef LearningRuleInterface interface;

	static void __calculateDynamicsDefault(const Time &t) {}
	static void __propagateSynapseSpikeDefault(const SynSpike &s) {}

	static void provideDefaultInterface(LearningRuleInterface &i) {
		i.calculateDynamics = &LearningRuleBase::__calculateDynamicsDefault;
		i.propagateSynapseSpike =  &LearningRuleBase::__propagateSynapseSpikeDefault;
	}
	virtual void provideInterface(LearningRuleInterface &i) = 0;
	virtual void propagateSynapseSpike(const SynSpike &s) = 0;
	virtual void calculateDynamics(const Time &t) = 0;
	
	Statistics& getStat() {
		return stat;
	}
	void linkWithNeuron(SpikeNeuronBase *_n) {
		n = _n;
	}
protected:
	Statistics stat;
	SpikeNeuronBase *n;
};

template <typename Constants, typename State>
class LearningRule : public LearningRuleBase {
public:	
	void serial_process() {
		begin() << "Constants: " << c;
		if (messages->size() == 0) {
			(*this) << Self::end;
			return;
		}
		(*this) << "State: " << s << Self::end;
	}

protected:
	State s;
	Constants c;
};




}

