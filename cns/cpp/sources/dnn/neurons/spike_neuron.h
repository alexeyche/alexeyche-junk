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

};


template <typename Constants, typename State>
class SpikeNeuron : public SpikeNeuronBase {
public:
	// void setLearningRule(LearningRule *_lrule) { lrule = _lrule; }
	void setActFunction(ActFunctionBase *_act_f) { act_f.set(_act_f); }
	
	void serialize() {
		begin() << "State: "       << s << ", " \
			    << "Constants: "   << c << ", " \
			    << act_f.isSet() ? << "ActFunction: " << act_f.ref() << end();
	}


protected:
	vector<InterfacedPtr<SynapseBase>> syns;

	InterfacedPtr<ActFunctionBase> act_f;
	// InterfacedPtr<LearningRule> lrule;
	// InterfacedPtr<Input> input;
	// InterfacedPtr<TuningCurve> tc;

	State s;
    Constants c;
};


}pragma once


include <dnn/base/base.h>
include <dnn/util/interfaced_ptr.h>
include <dnn/act_functions/act_function.h>
include <dnn/synapses/synapse.h>
include <dnn/io/serialize.h>

amespace dnn {


truct SpikeNeuronInterface {
   calculateDynamicsDelegate calculateDynamics;
   propSynSpikeDelegate propagateSynapseSpike;
   getDoubleDelegate getFiringProbability;
   getBoolDelegate fired;
;


lass SpikeNeuronBase : public SerializableBase {
ublic:
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

;

ize_t global_neuron_index = 0;

emplate <typename Constants, typename State>
lass SpikeNeuron : public SpikeNeuronBase {
ublic:
   SpikeNeuron() {
   	_id = global_neuron_index++;
   }
   // void setLearningRule(LearningRule *_lrule) { lrule = _lrule; }
   void setActFunction(ActFunctionBase *_act_f) { act_f.set(_act_f); }
<<<<<< HEAD
   
   void serialize() {
   	begin() << "State: "       << s << ", " \
   		    << "Constants: "   << c << ", " \
   		    << act_f.isSet() ? << "ActFunction: " << act_f.ref() << end();
   }


======
   inline const size_t& id() { return _id; }

   void processStream(Stream &str) {
   	acquire(str) << s << c;
   }
>>>>>> 8a4fc40b4f3e6c4178eb87b6cfcbc44027f630b7
rotected:
   size_t _id;
   vector<InterfacedPtr<SynapseBase>> syns;

   InterfacedPtr<ActFunctionBase> act_f;
   // InterfacedPtr<LearningRule> lrule;
   // InterfacedPtr<Input> input;
   // InterfacedPtr<TuningCurve> tc;

   State s;
   Constants c;
;



