#pragma once


#include <dnn/base/base.h>
#include <dnn/util/interfaced_ptr.h>
#include <dnn/act_functions/act_function.h>
#include <dnn/synapses/synapse.h>
#include <dnn/io/serialize.h>

namespace dnn {


struct SpikeNeuronInterface {
	calculateDynamicsDelegate calculateDynamics;
	propSynSpikeDelegate propagateSynapseSpike;
	getDoubleDelegate getFiringProbability;
	getBoolDelegate fired;
};


class SpikeNeuronBase : public SerializableBase {
public:
	typedef SpikeNeuronInterface interface;

	virtual void provideInterface(SpikeNeuronInterface &i) = 0;

	virtual void reset() = 0;

	// runtime
	virtual void propagateSynapseSpike(const SynSpike &s) = 0;
	virtual void calculateDynamics(const Time &t) = 0;
	virtual const double& getFiringProbability() = 0;
	virtual const bool& fired() = 0;

	static void provideDefaultInterface(SpikeNeuronInterface &i) {
		cerr << "No default interface for SpikeNeuron\n";
		terminate();
	}

	// void setLearningRule(LearningRule *_lrule) { lrule = _lrule; }
	void setActFunction(ActFunctionBase *_act_f) { act_f.set(_act_f); }

protected:
	vector<InterfacedPtr<SynapseBase>> syns;

	InterfacedPtr<ActFunctionBase> act_f;
	// InterfacedPtr<LearningRule> lrule;
	// InterfacedPtr<Input> input;
	// InterfacedPtr<TuningCurve> tc;


};

/*@GENERATE_PROTO@*/
struct SpikeNeuronInfo : public Serializable<Protos::SpikeNeuronInfo> {
	void serial_process() {
		cout << "here0\n";
		begin() << "num_of_synapses: " << num_of_synapses << ", "\
				<< "act_function_is_set: " << act_function_is_set << end();
		cout << "here1\n";
	}

	size_t num_of_synapses;
	bool act_function_is_set;
};

template <typename Constants, typename State>
class SpikeNeuron : public SpikeNeuronBase {
public:
	SpikeNeuronInfo getInfo() {
		SpikeNeuronInfo info;
		info.num_of_synapses = syns.size();
		info.act_function_is_set = act_f.isSet();
		return info;
	}

	void serial_process() {
		SpikeNeuronInfo info;
		if(mode == ProcessingOutput) {
			info = getInfo();
		}
		begin() << "SpikeNeuronInfo: "   << info  << ", " \
				<< "Constants: "   		 << c 	  << ", " \
				<< "State: "       		 << s;
		if (info.act_function_is_set) {
			(*this) << "ActFunction: " << act_f;
		}
		for(size_t i=0; i<info.num_of_synapses; ++i) {
			(*this) << "Synapse: " << s;
		}
		end();
	}
protected:
	SpikeNeuronInfo info;
	State s;
    Constants c;
};


}
