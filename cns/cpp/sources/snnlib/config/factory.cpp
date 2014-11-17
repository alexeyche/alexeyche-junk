
#include "factory.h"

#include <snnlib/config/constants.h>
#include <snnlib/learning/optimal_stdp.h>
#include <snnlib/act_funcs/determ.h>
#include <snnlib/act_funcs/exp_hennequin.h>
#include <snnlib/layers/synapse.h>
#include <snnlib/tuning_curves/sigma_tuning_curve.h>
#include <snnlib/layers/srm_layer.h>
#include <snnlib/layers/neuron.h>



Factory& Factory::inst() {
    static Factory _inst;
    return _inst;
}


Factory::Factory() {
    const_map["SRMNeuron"]     =   &createInstance<Obj, SRMNeuronC>;
    const_map["SRMLayer"]     =   &createInstance<Obj, SRMLayerC>;
    const_map["Synapse"]      =   &createInstance<Obj, SynapseC>;
    const_map["Determ"]       =   &createInstance<Obj, DetermC>;
    const_map["ExpHennequin"] =   &createInstance<Obj, ExpHennequinC>;
    const_map["OptimalStdp"]  =   &createInstance<Obj, OptimalStdpC>;
    const_map["SigmaTuningCurve"]  =  &createInstance<Obj, SigmaTuningCurveC>;

    entity_map["SRMNeuron"]     =   &createInstance<Obj, SRMNeuron>;
    entity_map["SRMLayer"]     =   &createInstance<Obj, SRMLayer>;
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

ConstObj* Factory::createConst(string name, JsonBox::Value v) {
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

ActFunc* Factory::createActFunc(string name, const ConstObj *c) {
    GET_BASE_NAME(entity_map)
    ActFunc *o = dynamic_cast<ActFunc*>(entity_map[base_struct_name]());
    if(!o) { cerr << "Error while reading " << name << " and treating like ActFunc\n"; terminate(); }
    o->init(c);
    objects.push_back(o);
    return o;
}

LearningRule*  Factory::createLearningRule(string name, const ConstObj *c) {
    GET_BASE_NAME(entity_map)
    LearningRule *o = dynamic_cast<LearningRule*>(entity_map[base_struct_name]());
    if(!o) { cerr << "Error while reading " << name << " and treating like LearningRule\n"; terminate(); }
    o->init(c);
    objects.push_back(o);
    return o;
}

Synapse*  Factory::createSynapse(string name, const ConstObj *c, size_t id_pre, double w) {
    GET_BASE_NAME(entity_map)
    Synapse *o = dynamic_cast<Synapse*>(entity_map[base_struct_name]());
    if(!o) { cerr << "Error while reading " << name << " and treating like Synapse\n"; terminate(); }
    o->init(c, id_pre, w);
    objects.push_back(o);
    return o;
}

Neuron* Factory::createNeuron(string name, const ConstObj *c, ActFunc *act, LearningRule *lr, TuningCurve *tc) {
    GET_BASE_NAME(entity_map)
    Neuron *o = dynamic_cast<Neuron*>(entity_map[base_struct_name]());
    if(!o) { cerr << "Error while reading " << name << " and treating like Neuron\n"; terminate(); }
    o->init(c, act, lr, tc);
    objects.push_back(o);
    return o;
}

Layer* Factory::createLayer(string name, const ConstObj* c, size_t size, const NeuronConf &nc, const Constants &glob_c) {
    GET_BASE_NAME(entity_map)
    Layer *o = dynamic_cast<Layer*>(entity_map[base_struct_name]());
    if(!o) { cerr << "Error while reading " << name << " and treating like Layer\n"; terminate(); }
    o->init(size, c, nc, glob_c);
    objects.push_back(o);
    return o;
}


TuningCurve* Factory::createTuningCurve(string name, const ConstObj *c) {
    GET_BASE_NAME(entity_map)
    TuningCurve *o = dynamic_cast<TuningCurve*>(entity_map[base_struct_name]());
    if(!o) { cerr << "Error while reading " << name << " and treating like TuningCurve\n"; terminate(); }
    o->init(c);
    objects.push_back(o);
    return o;
}

