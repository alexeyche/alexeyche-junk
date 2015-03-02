


#include <dnn/base/base.h>
#include <dnn/neurons/leaky_integrate_and_fire.h>
#include <dnn/act_functions/determ.h>


#include <dnn/io/serialize.h>

#include "factory.h"

namespace dnn {

Factory::entity_map_type Factory::typemap;

Factory& Factory::inst() {
    static Factory _inst;
    return _inst;
}


#define REG_TYPE(name) \
	registerType<name>(#name);\

#define REG_TYPE_WITH_CONST(name) \
	registerType<name>(#name);\
	registerType<name##C>(string(#name) + string("C"));\
	
#define REG_TYPE_WITH_STATE_AND_CONST(name) \
	registerType<name>(#name);\
	registerType<name##C>(string(#name) + string("C"));\
	registerType<name##State>(string(#name) + string("State"));\


Factory::Factory() {
	REG_TYPE_WITH_STATE_AND_CONST(LeakyIntegrateAndFire);
	REG_TYPE_WITH_CONST(Determ);
}

Factory::~Factory() {
	for(auto o: objects) {
		delete o;
	}
}

SerializableBase* Factory::createObject(string name) {
	SerializableBase* o = typemap[name]();
	objects.push_back(o);
	return o;
}

SpikeNeuronBase* Factory::createSpikeNeuron(string name) {
	SerializableBase *b = createObject(name);
	SpikeNeuronBase *p = dynamic_cast<SpikeNeuronBase*>(b);
	if(!p) {
	    cerr << "Error to cast " << b->name() << " to SpikeNeuronBase" << "\n";
	    terminate();
	}
	return p;
}

ActFunctionBase* Factory::createActFunction(string name) {
	SerializableBase *b = createObject(name);
	ActFunctionBase *p = dynamic_cast<ActFunctionBase*>(b);
	if(!p) {
	    cerr << "Error to cast " << b->name() << " to ActFunctionBase" << "\n";
	    terminate();
	}
	return p;
}



}