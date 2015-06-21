#pragma once

#include <dnn/io/serialize.h>
#include <dnn/util/statistics.h>
#include <dnn/neurons/spike_neuron.h>
#include <dnn/util/ptr.h>

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
	
	template <typename T>
	void provideInterface(LearningRuleInterface &i) {
        i.calculateDynamics = MakeDelegate(static_cast<T*>(this), &T::calculateDynamics);
        i.propagateSynapseSpike = MakeDelegate(static_cast<T*>(this), &T::propagateSynapseSpike);
    }

	virtual void propagateSynapseSpike(const SynSpike &s) = 0;
	virtual void calculateDynamics(const Time &t) = 0;
	virtual void reset() = 0;
		
	Statistics& getStat() {
		return stat;
	}
	virtual void linkWithNeuron(SpikeNeuronBase &_n) = 0;
protected:
	Statistics stat;
};

template <typename Constants, typename State, typename Neuron>
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

	void linkWithNeuron(Neuron &_n) {
		n.set(_n);
	}
protected:
	Ptr<Neuron> n;

	State s;
	Constants c;
};




}

