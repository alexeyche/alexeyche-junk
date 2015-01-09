
#include "factory.h"

#include <snnlib/layers/layer.h>
#include <snnlib/config/constants.h>
#include <snnlib/learning/optimal_stdp.h>
#include <snnlib/learning/max_likelihood.h>
#include <snnlib/weight_normalizations/min_max.h>
#include <snnlib/weight_normalizations/soft_min_max.h>
#include <snnlib/weight_normalizations/nonlinear_min_max.h>
#include <snnlib/weight_normalizations/mean_activity_homeostasis.h>
#include <snnlib/learning/stdp.h>
#include <snnlib/learning/triple_stdp.h>
#include <snnlib/reinforcement/likelihood.h>
#include <snnlib/reinforcement/input_classification.h>
#include <snnlib/act_funcs/determ.h>
#include <snnlib/act_funcs/exp_hennequin.h>
#include <snnlib/neurons/synapse.h>
#include <snnlib/tuning_curves/sigma_tuning_curve.h>
#include <snnlib/neurons/srm_neuron.h>
#include <snnlib/neurons/adex_neuron.h>
#include <snnlib/neurons/neuron.h>
#include <snnlib/sim/runtime_globals.h>
#include <snnlib/sim/sim.h>

#include <snnlib/util/spikes_list.h>
#include <snnlib/util/matrix.h>
#include <snnlib/util/time_series.h>

Factory& Factory::inst() {
    static Factory _inst;
    return _inst;
}


Factory::Factory() {
    const_map["SRMNeuron"]     =   &createConstInstance<Obj, SRMNeuronC>;
    const_map["AdExNeuron"]     =   &createConstInstance<Obj, AdExNeuronC>;
    const_map["Synapse"]      =   &createConstInstance<Obj, SynapseC>;
    const_map["Determ"]       =   &createConstInstance<Obj, DetermC>;
    const_map["ExpHennequin"] =   &createConstInstance<Obj, ExpHennequinC>;
    const_map["OptimalStdp"]  =   &createConstInstance<Obj, OptimalStdpC>;
    const_map["MaxLikelihood"]  =   &createConstInstance<Obj, MaxLikelihoodC>;
    const_map["Likelihood"]  =   &createConstInstance<Obj, LikelihoodC>;
    const_map["SigmaTuningCurve"]  =  &createConstInstance<Obj, SigmaTuningCurveC>;
    const_map["InputClassification"]  =  &createConstInstance<Obj, InputClassificationC>;
    const_map["MeanActivityHomeostasis"]  =  &createConstInstance<Obj, MeanActivityHomeostasisC>;
    const_map["MinMax"]  =  &createConstInstance<Obj, MinMaxC>;
    const_map["SoftMinMax"]  =  &createConstInstance<Obj, SoftMinMaxC>;
    const_map["NonlinearMinMax"]  =  &createConstInstance<Obj, NonlinearMinMaxC>;
    const_map["Stdp"]  =  &createConstInstance<Obj, StdpC>;
    const_map["TripleStdp"]  =  &createConstInstance<Obj, TripleStdpC>;
    const_map["TripleStdpMin"]  =  &createConstInstance<Obj, TripleStdpC>;

    entity_map["SRMNeuron"]     =   &createInstance<Obj, SRMNeuron>;
    entity_map["AdExNeuron"]     =   &createInstance<Obj, AdExNeuron>;
    entity_map["Synapse"]      =   &createInstance<Obj, Synapse>;
    entity_map["Determ"]       =   &createInstance<Obj, Determ>;
    entity_map["ExpHennequin"] =   &createInstance<Obj, ExpHennequin>;
    entity_map["OptimalStdp"]  =   &createInstance<Obj, OptimalStdp>;
    entity_map["SigmaTuningCurve"]  =  &createInstance<Obj, SigmaTuningCurve>;
    entity_map["MaxLikelihood"]  =   &createInstance<Obj, MaxLikelihood>;
    entity_map["Likelihood"]  =   &createInstance<Obj, Likelihood>;
    entity_map["InputClassification"]  =   &createInstance<Obj, InputClassification>;
    entity_map["MeanActivityHomeostasis"]  =  &createInstance<Obj, MeanActivityHomeostasis>;
    entity_map["MinMax"]  =  &createInstance<Obj, MinMax>;
    entity_map["SoftMinMax"]  =  &createInstance<Obj, SoftMinMax>;
    entity_map["NonlinearMinMax"]  =  &createInstance<Obj, NonlinearMinMax>;
    entity_map["Stdp"]  =  &createInstance<Obj, Stdp>;
    entity_map["TripleStdp"]  =  &createInstance<Obj, TripleStdp>;    
    entity_map["TripleStdpMin"]  =  &createInstance<Obj, TripleStdp>;    

}

#define GET_BASE_NAME(map) \
    string base_struct_name(name);\
    auto it = map.find(name);\
    if(it == map.end()) { \
        base_struct_name = findBaseStructName(name);\
    }\


SerializableBase* Factory::createSerializable(const string &name) {
    SerializableBase *s = nullptr;
    if(name == "AdExNeuronStat") {
        s = new AdExNeuronStat();
    } else
    if(name == "NeuronStat") {
        s = new NeuronStat();
    } else
    if(name == "SpikesList") {
        s = new SpikesList();
    } else
    if(name == "Synapse") {
        s = new Synapse();
    } else
    if(name == "AdExNeuron") {
        s = new AdExNeuron();
    } else
    if(name == "SRMNeuron") {
        s = new SRMNeuron();
    } else
    if(name == "MaxLikelihoodStat") {
        s = new MaxLikelihoodStat();
    } else
    if(name == "Constants") {
        s = new Constants();
    } else
    if(name == "OptimalStdp") {
        s = new OptimalStdp();
    } else
    if(name == "DoubleMatrix") {
        s = new DoubleMatrix();
    } else
    if(name == "Neuron") {
        cerr << "Neuron is not allow to be created by serialization factory\n";
        terminate();
    } else
    if(name == "LabeledTimeSeries") {
        s = new LabeledTimeSeries();
    } else
    if(name == "RewardStat") {
        s = new RewardStat();
    } else
    if(name == "MaxLikelihood") {
        s = new MaxLikelihood();
    } else
    if(name == "InputClassification") {
        s = new InputClassification();
    } else
    if(name == "Reward") {
        s = new Reward();
    } else
    if(name == "LabeledSpikesList") {
        s = new LabeledSpikesList();
    } else
    if(name == "Sim") {
        s = new Sim();
    } else
    if(name == "Stdp") {
        s = new Stdp();
    } else
    if(name == "TripleStdp") {
        s = new TripleStdp();
    } else
    if(name == "TripleStdpStat") {
        s = new TripleStdpStat();
    } else
    if(name == "StdpStat") {
        s = new StdpStat();
    } else
    if(name == "OptimalStdpStat") {
        s = new OptimalStdpStat();
    } else
    if(name == "LabeledTimeSeriesList") {
        s = new LabeledTimeSeriesList();
    } else {
        cerr << "Unknown Serializable name " << name << "\n";
        terminate();
    }
    objects.push_back(s);
    return s;
}


ConstObj *Factory::createConst(string name, JsonBox::Value v) {
    GET_BASE_NAME(const_map)
    ConstObj *o = dynamic_cast<ConstObj*>(const_map[base_struct_name](name));
    if(!o) {
        cerr << "Error while reading " << name << " and treating like ConstObj\n";
        terminate();
    }
    o->fill_structure(v);
    objects.push_back(o);
    return o;
}

ActFunc *Factory::createActFunc(string name, const Constants &c, Neuron *n) {
    GET_BASE_NAME(entity_map)
    ActFunc *o = dynamic_cast<ActFunc*>(entity_map[base_struct_name]());
    if(!o) { cerr << "Error while reading " << name << " and treating like ActFunc\n"; terminate(); }
    o->init(c[name], n);
    objects.push_back(o);
    return o;
}


LearningRule * Factory::createLearningRule(string name, const Constants &c, Neuron *n, ActFunc *act_f, WeightNormalization *wnorm) {
    GET_BASE_NAME(entity_map)
    LearningRule *o = dynamic_cast<LearningRule*>(entity_map[base_struct_name]());
    if(!o) { cerr << "Error while reading " << name << " and treating like LearningRule\n"; terminate(); }
    o->init(c[name], n, act_f, wnorm);
    objects.push_back(o);
    return o;
}

Synapse *Factory::createSynapse(string name, const Constants &c, size_t id_pre, double w, double dendrite_delay) {
    GET_BASE_NAME(entity_map)
    Synapse *o = dynamic_cast<Synapse*>(entity_map[base_struct_name]());
    if(!o) { cerr << "Error while reading " << name << " and treating like Synapse\n"; terminate(); }
    o->init(c[name], id_pre, w, dendrite_delay);
    dyn_objects.push_back(o);
    return o;
}
RewardModulation* Factory::createRewardModulation(string name, const Constants &c, Neuron *n, RuntimeGlobals *run_glob_c) {
    GET_BASE_NAME(entity_map)
    RewardModulation *o = dynamic_cast<RewardModulation*>(entity_map[base_struct_name]());
    if(!o) { cerr << "Error while reading " << name << " and treating like RewardModulation\n"; terminate(); }
    o->init(c[name], n, run_glob_c);
    objects.push_back(o);
    return o;
}

WeightNormalization* Factory::createWeightNormalization(string name, const Constants &c, Neuron *n) {
    GET_BASE_NAME(entity_map)
    WeightNormalization *o = dynamic_cast<WeightNormalization*>(entity_map[base_struct_name]());
    if(!o) { cerr << "Error while reading " << name << " and treating like WeightNormalization\n"; terminate(); }
    o->init(c[name], n);
    objects.push_back(o);
    return o;
}


Neuron *Factory::createNeuron(string name, size_t local_id, const Constants &c, const RuntimeGlobals *run_glob_c, double axon_delay) {
    GET_BASE_NAME(entity_map)
    Neuron *o = dynamic_cast<Neuron*>(entity_map[base_struct_name]());
    if(!o) { cerr << "Error while reading " << name << " and treating like Neuron\n"; terminate(); }
    o->init(c[name], local_id, run_glob_c, axon_delay);
    objects.push_back(o);
    return o;
}

Layer *Factory::createLayer(size_t size, bool wta, const NeuronConf &nc, const Constants &c, RuntimeGlobals *run_glob_c, bool learning = true) {
    Layer *o = new Layer();
    o->init(size, wta, nc, c, run_glob_c, learning); 
    objects.push_back(o);
    return o;
}


TuningCurve *Factory::createTuningCurve(string name, const Constants &c,  size_t layer_size, size_t neuron_id, Neuron *n) {
    GET_BASE_NAME(entity_map)
    TuningCurve *o = dynamic_cast<TuningCurve*>(entity_map[base_struct_name]());
    if(!o) { cerr << "Error while reading " << name << " and treating like TuningCurve\n"; terminate(); }
    o->init(c[name], layer_size, neuron_id, n);
    objects.push_back(o);
    return o;
}


