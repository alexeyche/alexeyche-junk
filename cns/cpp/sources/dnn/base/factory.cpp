
#include <dnn/base/base.h>
#include <dnn/neurons/leaky_integrate_and_fire.h>
#include <dnn/neurons/adapt_integrate_and_fire.h>
#include <dnn/neurons/spike_sequence_neuron.h>
#include <dnn/neurons/srm_neuron.h>
#include <dnn/act_functions/determ.h>
#include <dnn/act_functions/exp_threshold.h>
#include <dnn/synapses/static_synapse.h>
#include <dnn/synapses/std_synapse.h>
#include <dnn/inputs/input_time_series.h>
#include <dnn/io/serialize.h>
#include <dnn/util/time_series.h>
#include <dnn/util/statistics.h>
#include <dnn/util/matrix.h>
#include <dnn/mpl/mpl.h>
#include <dnn/util/spikes_list.h>
#include <dnn/learning_rules/stdp.h>
#include <dnn/learning_rules/stdp_time.h>

#include <dnn/connections/stochastic.h>
#include <dnn/connections/difference_of_gaussians.h>

#include "factory.h"

namespace dnn {

Factory::entity_map_type Factory::typemap;
Factory::proto_map_type Factory::prototypemap;

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


Factory::Factory() : registration_is_on(true) {
	REG_TYPE(SpikeNeuronInfo);
	REG_TYPE(SynapseInfo);
	
	REG_TYPE_WITH_STATE_AND_CONST(LeakyIntegrateAndFire);
	REG_TYPE_WITH_STATE_AND_CONST(AdaptIntegrateAndFire);
	REG_TYPE_WITH_STATE_AND_CONST(StaticSynapse);
	REG_TYPE_WITH_STATE_AND_CONST(STDSynapse);
	REG_TYPE_WITH_STATE_AND_CONST(SpikeSequenceNeuron);
	REG_TYPE_WITH_STATE_AND_CONST(SRMNeuron);
	REG_TYPE_WITH_CONST(Determ);
	REG_TYPE_WITH_CONST(ExpThreshold);
	REG_TYPE_WITH_CONST(Stochastic);
	REG_TYPE_WITH_CONST(DifferenceOfGaussians);
	REG_TYPE_WITH_STATE_AND_CONST(InputTimeSeries);
	REG_TYPE_WITH_STATE_AND_CONST(Stdp);
	REG_TYPE_WITH_STATE_AND_CONST(StdpTime);
	
	REG_TYPE(Statistics);
	REG_TYPE(StatisticsInfo);
	REG_TYPE(Stat);
	REG_TYPE(SpikesList);
	REG_TYPE(SpikesListInfo);
	REG_TYPE(SpikesSequence);
	
	REG_TYPE(TimeSeries);
	REG_TYPE(TimeSeriesInfo);
	REG_TYPE(TimeSeriesDimInfo);
	REG_TYPE(TimeSeriesData);

	REG_TYPE(DoubleMatrix);
	REG_TYPE(MatchingPursuitConfig)
	REG_TYPE(FilterMatch);
}

Factory::~Factory() {
	for (auto &o : objects) {
		delete o;
	}
}


SerializableBase* Factory::createObject(string name) {
	if (typemap.find(name) == typemap.end()) {
		throw dnnException()<< "Failed to find method to construct type " << name << "\n";
	}
	SerializableBase* o = typemap[name]();
	if(registration_is_on) {
		objects.push_back(o);
		objects_map.insert(std::make_pair(o->name(), objects.size()-1));
	}
	return o;
}

ProtoMessage Factory::createProto(string name) {
	if (prototypemap.find(name) == prototypemap.end()) {
		throw dnnException()<< "Failed to find method to construct proto type " << name << "\n";
	}
	ProtoMessage o = prototypemap[name]();
	return o;
}



SerializableBase* Factory::getCachedObject(const string& filename) {	
    if(cache_map.find(filename) == cache_map.end()) {
        ifstream f(filename);
        Stream s(f, Stream::Binary);

        cache_map[filename] = s.readBaseObject();
    }
    return cache_map[filename];
}


pair<Factory::object_iter, Factory::object_iter> Factory::getObjectsSlice(const string& name) {
	return objects_map.equal_range(name);
}

}
