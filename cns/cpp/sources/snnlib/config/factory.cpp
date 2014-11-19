
#include "factory.h"

#include <snnlib/config/constants.h>
#include <snnlib/learning/optimal_stdp.h>
#include <snnlib/act_funcs/determ.h>
#include <snnlib/act_funcs/exp_hennequin.h>
#include <snnlib/layers/synapse.h>
#include <snnlib/tuning_curves/sigma_tuning_curve.h>
#include <snnlib/layers/srm_neuron.h>
#include <snnlib/layers/adex_neuron.h>
#include <snnlib/layers/neuron.h>



Factory& Factory::inst() {
    static Factory _inst;
    return _inst;
}


Factory::Factory() {
    const_map["Global"]     =   &createInstance<Obj, GlobalC>;
    const_map["SRMNeuron"]     =   &createInstance<Obj, SRMNeuronC>;
    const_map["AdExNeuron"]     =   &createInstance<Obj, AdExNeuronC>;
    const_map["Synapse"]      =   &createInstance<Obj, SynapseC>;
    const_map["Determ"]       =   &createInstance<Obj, DetermC>;
    const_map["ExpHennequin"] =   &createInstance<Obj, ExpHennequinC>;
    const_map["OptimalStdp"]  =   &createInstance<Obj, OptimalStdpC>;
    const_map["SigmaTuningCurve"]  =  &createInstance<Obj, SigmaTuningCurveC>;

    entity_map["SRMNeuron"]     =   &createInstance<Obj, SRMNeuron>;
    entity_map["AdExNeuron"]     =   &createInstance<Obj, AdExNeuron>;
    entity_map["Synapse"]      =   &createInstance<Obj, Synapse>;
    entity_map["Determ"]       =   &createInstance<Obj, Determ>;
    entity_map["ExpHennequin"] =   &createInstance<Obj, ExpHennequin>;
    entity_map["OptimalStdp"]  =   &createInstance<Obj, OptimalStdp>;
    entity_map["SigmaTuningCurve"]  =  &createInstance<Obj, SigmaTuningCurve>;

    // blank stuff
    entity_map["BlankTuningCurve"]  =  &createInstance<Obj, BlankTuningCurve>;
    entity_map["BlankLearningRule"]  =  &createInstance<Obj, BlankLearningRule>;
}

#define GET_BASE_NAME(map) \
    string base_struct_name(name);\
    auto it = map.find(name);\
    if(it == map.end()) { \
        base_struct_name = findBaseStructName(name);\
    }\

ConstObj *Factory::createConst(string name, JsonBox::Value v) {
    GET_BASE_NAME(const_map)
    ConstObj *o = dynamic_cast<ConstObj*>(const_map[base_struct_name]());
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

LearningRule * Factory::createLearningRule(string name, const Constants &c, Neuron *n) {
    GET_BASE_NAME(entity_map)
    LearningRule *o = dynamic_cast<LearningRule*>(entity_map[base_struct_name]());
    if(!o) { cerr << "Error while reading " << name << " and treating like LearningRule\n"; terminate(); }
    o->init(c[name], n);
    objects.push_back(o);
    return o;
}

Synapse *Factory::createSynapse(string name, const Constants &c, size_t id_pre, double w) {
    GET_BASE_NAME(entity_map)
    Synapse *o = dynamic_cast<Synapse*>(entity_map[base_struct_name]());
    if(!o) { cerr << "Error while reading " << name << " and treating like Synapse\n"; terminate(); }
    o->init(c[name], id_pre, w);
    objects.push_back(o);
    return o;
}

Neuron *Factory::createNeuron(string name, const Constants &c) {
    GET_BASE_NAME(entity_map)
    Neuron *o = dynamic_cast<Neuron*>(entity_map[base_struct_name]());
    if(!o) { cerr << "Error while reading " << name << " and treating like Neuron\n"; terminate(); }
    o->init(c[name]);
    vector<string> dep_c = o->getDependentConstantsNames();
    if(dep_c.size()>0) {
        vector<const ConstObj *> provided_constants;
        for(auto it=dep_c.begin(); it != dep_c.end(); ++it) {
            provided_constants.push_back( c[*it] );                
        }
        o->setDependentConstants(provided_constants); 
    }
    objects.push_back(o);
    return o;
}

Layer *Factory::createLayer(size_t size, const NeuronConf &nc, const Constants &c) {
    Layer *o = new Layer();
    o->init(size, nc, c);
    objects.push_back(o);
    return o;
}


TuningCurve *Factory::createTuningCurve(string name, const Constants &c, Neuron *n) {
    GET_BASE_NAME(entity_map)
    TuningCurve *o = dynamic_cast<TuningCurve*>(entity_map[base_struct_name]());
    if(!o) { cerr << "Error while reading " << name << " and treating like TuningCurve\n"; terminate(); }
    o->init(c[name], n);
    objects.push_back(o);
    return o;
}

