#pragma once


#include <dnn/base/base.h>
#include <dnn/util/interfaced_ptr.h>
#include <dnn/act_functions/act_function.h>
#include <dnn/synapses/synapse.h>
#include <dnn/inputs/input.h>
#include <dnn/io/serialize.h>
#include <dnn/util/statistics.h>

namespace dnn {


struct SpikeNeuronInterface {
	calculateDynamicsDelegate calculateDynamics;
	propSynSpikeDelegate propagateSynapseSpike;
	getDoubleDelegate getFiringProbability;
	getBoolCopyDelegate pullFiring;
};
extern size_t global_neuron_index;

class Builder;

class SpikeNeuronBase : public SerializableBase {
friend class Builder;
friend class Sim;
public:
	SpikeNeuronBase() {
		_id = global_neuron_index++;
	}

	typedef SpikeNeuronInterface interface;

	inline const size_t& id() {
		return _id;
	}
	inline const double& axonDelay() {
		return axon_delay;
	}
	void setAxonDelay(double d) {
		axon_delay = d;
	}

	virtual void provideInterface(SpikeNeuronInterface &i) = 0;

	virtual void reset() = 0;

	// runtime
	virtual void propagateSynapseSpike(const SynSpike &s) = 0;
	virtual void calculateDynamics(const Time &t) = 0;
	virtual const double& getFiringProbability() = 0;
	virtual bool pullFiring() = 0;


	static void __calculateDynamicsDefault(const Time &t) {
		cerr << "Calling inapropriate default interface function\n";
		terminate();
	}
	static void __propagateSynapseSpikeDefault(const SynSpike &s) {
		cerr << "Calling inapropriate default interface function\n";
		terminate();
	}
	static const double& __getFiringProbabilityDefault() {
		cerr << "Calling inapropriate default interface function\n";
		terminate();
	}
	static bool __pullFiringDefault() {
		cerr << "Calling inapropriate default interface function\n";
		terminate();
	}
	static void provideDefaultInterface(SpikeNeuronInterface &i) {
		i.calculateDynamics = &SpikeNeuronBase::__calculateDynamicsDefault;
		i.pullFiring = &SpikeNeuronBase::__pullFiringDefault;
		i.getFiringProbability = &SpikeNeuronBase::__getFiringProbabilityDefault;
		i.propagateSynapseSpike =  &SpikeNeuronBase::__propagateSynapseSpikeDefault;
	}

	// void setLearningRule(LearningRule *_lrule) { lrule = _lrule; }
	void setActFunction(ActFunctionBase *_act_f) { act_f.set(_act_f); }
	
	void setInput(InputBase *_input) { input.set(_input); }
	bool inputIsSet() {
		return input.isSet();
	}
	InputBase& getInput() {
		if(!input.isSet()) {
			cerr << "Trying to get input which is not set\n";
			terminate();
		}
		return input.ref();
	}
	
	void addSynapse(InterfacedPtr<SynapseBase> syn) {
		syns.push_back(syn);
	}
	inline vector<InterfacedPtr<SynapseBase>>& getSynapses() {
		return syns;
	}

protected:
	size_t _id;
	double axon_delay;
	vector<InterfacedPtr<SynapseBase>> syns;

	InterfacedPtr<ActFunctionBase> act_f;
	InterfacedPtr<InputBase> input;

	Statistics stat;
};

/*@GENERATE_PROTO@*/
struct SpikeNeuronInfo : public Serializable<Protos::SpikeNeuronInfo> {
	void serial_process() {
		begin() << "id: " << id << ", " \
		        << "axon_delay: " << axon_delay << ", " \
		        << "num_of_synapses: " << num_of_synapses << ", " \
		        << "act_function_is_set: " << act_function_is_set << ", " \
		        << "input_is_set: " << input_is_set << Self::end;
	}
	size_t id;
	double axon_delay;
	size_t num_of_synapses;
	bool act_function_is_set;
	bool input_is_set;
};

template <typename Constants, typename State>
class SpikeNeuron : public SpikeNeuronBase {
public:
	SpikeNeuronInfo getInfo() {
		SpikeNeuronInfo info;
		info.id = id();
		info.axon_delay = axonDelay();
		info.num_of_synapses = syns.size();
		info.act_function_is_set = act_f.isSet();
		info.input_is_set = input.isSet();
		return info;
	}

	void serial_process() {
		begin() << "Constants: " << c;

		if (messages->size() == 0) {
			(*this) << Self::end;
			return;
		}

		(*this) << "State: " << s;

		if (messages->size() == 0) {
			(*this) << Self::end;
			return;
		}

		SpikeNeuronInfo info;
		if (mode == ProcessingOutput) {
			info = getInfo();
		}

		(*this) << "SpikeNeuronInfo: "   << info;

		if (info.act_function_is_set) {
			(*this) << "ActFunction: " << act_f;
		}
		if (info.input_is_set) {
			(*this) << "Input: " << input;
		}
		for (size_t i = 0; i < info.num_of_synapses; ++i) {
			(*this) << "Synapse: " << syns[i];
		}
		(*this) << Self::end;
	}

protected:
	SpikeNeuronInfo info;
	State s;
	Constants c;
};


}
