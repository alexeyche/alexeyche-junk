#pragma once

#include <snnlib/config/constants.h>
#include <snnlib/layers/layer.h>

typedef map<string, Entity*(*)()> entity_map_type;
typedef map<string, ConstObj*(*)()> const_map_type;

template<typename E,typename T> E* createInstance() { return new T; }

const_map_type generateConstMapType() {
    const_map_type m;
    
    m["IaFLayer"]     =   &createInstance<ConstObj, IaFLayerC>;
    m["Synapse"]      =   &createInstance<ConstObj, SynapseC>;
    m["Determ"]       =   &createInstance<ConstObj, DetermC>;
    m["ExpHennequin"] =   &createInstance<ConstObj, ExpHennequinC>;
    m["OptimalStdp"]  =   &createInstance<ConstObj, OptimalStdpC>;
    m["SigmaTCLayer"]  =  &createInstance<ConstObj, SigmaTCLayerC>;

    return m;
}

entity_map_type generateEntityMapType() {
    entity_map_type m;
    m["IaFLayer"] = &createInstance<Entity, IaFLayer<IaFNeuron> >;
//    m["Synapse"]      =   &createEntityInstance<Synapse>;
//    m["Determ"]       =   &createEntityInstance<Determ>;
//    m["ExpHennequin"] =   &createEntityInstance<ExpHennequin>;
//    m["OptimalStdp"]  =   &createEntityInstance<OptimalStdp>;
//    m["SigmaTCLayer"]  =  &createEntityInstance<SigmaTCLayer>;

    return m;
}

