


#include <dnn/base/base.h>
#include <dnn/neurons/leaky_integrate_and_fire.h>
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

#define REG_TYPE_WITH_STATE_AND_CONST(name) \
	registerType<name>(#name);\
	registerType<name##C>(string(#name) + string("C"));\
	registerType<name##State>(string(#name) + string("State"));\


Factory::Factory() {
	REG_TYPE_WITH_STATE_AND_CONST(LeakyIntegrateAndFire);
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





}