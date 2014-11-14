
#include "factory.h"

#include <snnlib/config/constants.h>
#include <snnlib/learning/optimal_stdp.h>
#include <snnlib/layers/act_funcs/determ.h>
#include <snnlib/layers/act_funcs/exp_hennequin.h>
#include <snnlib/layers/synapse.h>
#include <snnlib/layers/sigma_tc_layer.h>

Factory::Factory() {
    const_map["SRMLayer"]     =   &createInstance<Obj, SRMLayerC>;
    const_map["Synapse"]      =   &createInstance<Obj, SynapseC>;
    const_map["Determ"]       =   &createInstance<Obj, DetermC>;
    const_map["ExpHennequin"] =   &createInstance<Obj, ExpHennequinC>;
    const_map["OptimalStdp"]  =   &createInstance<Obj, OptimalStdpC>;
    const_map["SigmaTCLayer"]  =  &createInstance<Obj, SigmaTCLayerC>;

    entity_map["SRMLayer"]     =   &createInstance<Obj, SRMLayer<SRMNeuron> >;
    entity_map["Synapse"]      =   &createInstance<Obj, Synapse>;
    entity_map["Determ"]       =   &createInstance<Obj, Determ>;
    entity_map["ExpHennequin"] =   &createInstance<Obj, ExpHennequin>;
    entity_map["OptimalStdp"]  =   &createInstance<Obj, OptimalStdp>;
    entity_map["SigmaTCLayer"]  =  &createInstance<Obj, SigmaTCLayer>;
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
    return o;
}

ActFunc* Factory::createActFunc(string name, const ConstObj *c) {
    GET_BASE_NAME(entity_map)
    ActFunc *o = dynamic_cast<ActFunc*>(entity_map[base_struct_name]());
    if(!o) { cerr << "Error while reading " << name << " and treating like ActFunc\n"; terminate(); }
    return o;
}

LearningRule*  Factory::createLearningRule(string name, const ConstObj *c) {
    GET_BASE_NAME(entity_map)
    LearningRule *o = dynamic_cast<LearningRule*>(entity_map[base_struct_name]());
    if(!o) { cerr << "Error while reading " << name << " and treating like LearningRule\n"; terminate(); }
    return o;
}

LayerObj* Factory::createNetLayer(string name, size_t id, size_t size, const ConstObj *c, const ActFunc *act, const LearningRule *lrule) {
    GET_BASE_NAME(entity_map)
    LayerObj *o = dynamic_cast<LayerObj*>(entity_map[base_struct_name]());
    if(!o) { cerr << "Error while reading " << name << " and treating like LayerObj\n"; terminate(); }
    o->init(id, size, c, act, lrule);
    return o;
}

LayerObj* Factory::createInputLayer(string name, size_t id, size_t size, const ConstObj *c) {
    GET_BASE_NAME(entity_map)
    LayerObj *o = dynamic_cast<LayerObj*>(entity_map[base_struct_name]());
    if(!o) { cerr << "Error while reading " << name << " and treating like LayerObj\n"; terminate(); }
//    o->init(id, size, c, act, lrule);
    return o;
}
