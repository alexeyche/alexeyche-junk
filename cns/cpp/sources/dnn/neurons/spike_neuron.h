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


template <typename Constants, typename State>
class SpikeNeuron : public SpikeNeuronBase {
public:	
	void serialize() {
		begin() << "Constants: "   << c << ", " \
				<< "State: "       << s << ", " \
			    << "ActFunction: " << act_f << end();
	}
protected:
	
	State s;
    Constants c;
};


}
