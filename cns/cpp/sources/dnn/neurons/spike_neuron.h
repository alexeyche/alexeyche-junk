#pragma once

#include <dnn/base.h>
#include <dnn/util/interfaced_ptr.h>


struct SpikeNeuronInterface {
	stateDelegate calculate_dynamics;
	propSynSpikeDelegate propagate_synapse_spike;
	getBoolDelegate fired;
	getDoubleDelegate getFiringProbability;
};

class SpikeNeuronBase {
public:
	typedef SpikeNeuronInterface interface;

	virtual void provideInterface(SpikeNeuronInterface &i) = 0;

	static void provideDefaultInterface(SpikeNeuronInterface &i) {
		cerr << "No default interface for SpikeNeuron\n";
		terminate();
	}
};


template <typename Constants, typename State>
class SpikeNeuron : public SpikeNeuronBase {
public:
	SpikeNeuron(const Constants _c) : c(_c) : lrule(nullptr), act_f(nullptr), input(nullptr), tc(nullptr) {}
	
	// void setLearningRule(LearningRule *_lrule) { lrule = _lrule; }
	void setActFunction(ActFunctionBase *_act_f) { act_f.set(_act_f); }
	

	virtual void reset() = 0;	
	
	// runtime
	virtual void propagateSynapseSpike(const SynSpike &s) = 0;
	virtual void calculateDynamics() = 0;
	
protected:
	vector<InterfacedPtr<SynapseBase>> syns;

	InterfacedPtr<ActFunctionBase> act_f;
	// InterfacedPtr<LearningRule> lrule;
	// InterfacedPtr<Input> input;
	// InterfacedPtr<TuningCurve> tc;

	const Constants c;
	State s;
};
