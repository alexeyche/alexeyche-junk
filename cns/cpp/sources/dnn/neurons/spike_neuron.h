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
	getBoolDelegate fired;
};
extern size_t global_neuron_index;

class Builder;
class Network;

class SpikeNeuronBase : public SerializableBase {
friend class Builder;
friend class Network;
public:
	SpikeNeuronBase() : input_queue_lock(ATOMIC_FLAG_INIT) {
		_id = global_neuron_index++;
	}

	typedef SpikeNeuronInterface interface;

	inline const size_t& id() const {
		return _id;
	}
	inline const double& axonDelay() {
		return axon_delay;
	}

	virtual void provideInterface(SpikeNeuronInterface &i) = 0;

	virtual void reset() = 0;

	// runtime
	virtual void propagateSynapseSpike(const SynSpike &s) = 0;
	virtual void calculateDynamics(const Time &t) = 0;
	virtual const double& getFiringProbability() = 0;
	virtual const bool& fired() = 0;


	static void __calculateDynamicsDefault(const Time &t) {
		throw dnnException()<< "Calling inapropriate default interface function\n";
	}
	static void __propagateSynapseSpikeDefault(const SynSpike &s) {
		throw dnnException()<< "Calling inapropriate default interface function\n";
	}
	static const double& __getFiringProbabilityDefault() {
		throw dnnException()<< "Calling inapropriate default interface function\n";
	}
	static const bool& __firedDefault() {
		throw dnnException()<< "Calling inapropriate default interface function\n";
	}
	static void provideDefaultInterface(SpikeNeuronInterface &i) {
		i.calculateDynamics = &SpikeNeuronBase::__calculateDynamicsDefault;
		i.fired = &SpikeNeuronBase::__firedDefault;
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
			throw dnnException()<< "Trying to get input which is not set\n";
		}
		return input.ref();
	}
	
	void addSynapse(InterfacedPtr<SynapseBase> syn) {
		syns.push_back(syn);
	}
	inline vector<InterfacedPtr<SynapseBase>>& getSynapses() {
		return syns;
	}
	Statistics& getStat() {
		size_t syn_id = 0;
		for(auto &s: syns) {
			if(s.ref().getStat().on()) {
				Statistics& syn_st = s.ref().getStat();
				for(auto it=syn_st.getStats().begin(); it != syn_st.getStats().end(); ++it) {
					stat.getStats()[it->first + std::to_string(syn_id)] = it->second;	
				}						
			}
			syn_id++;
		}
		return stat;
	}

	inline void enqueueSpike(const SynSpike && sp) {
		while (input_queue_lock.test_and_set(std::memory_order_acquire));
		input_spikes.push(sp);
		input_queue_lock.clear(std::memory_order_release);
	}

	inline void readInputSpikes(const Time &t) {
		while (input_queue_lock.test_and_set(std::memory_order_acquire)) {}
        while(!input_spikes.empty()) {
            const SynSpike& sp = input_spikes.top();
            if(sp.t >= t.t) break;
            syns[ sp.syn_id ].ifc().propagateSpike();
            input_spikes.pop();   
        }
        input_queue_lock.clear(std::memory_order_release);        
	}
protected:
	size_t _id;
	double axon_delay;
	vector<InterfacedPtr<SynapseBase>> syns;

	InterfacedPtr<ActFunctionBase> act_f;
	InterfacedPtr<InputBase> input;

	Statistics stat;

	priority_queue<SynSpike> input_spikes;
	std::atomic_flag input_queue_lock;
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
		if(mode == ProcessingInput) {
			syns.resize(info.num_of_synapses);
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
