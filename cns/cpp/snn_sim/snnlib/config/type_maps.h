#pragma once

#include <snnlib/config/constants.h>

typedef map<string, const_element_t*(*)()> map_type;

template<typename T> const_element_t * createInstance() { return new T; }

map_type generateMapType() {
    map_type map;
    
    map["IaFLayer"]     =   &createInstance<IaFLayerC>;
    map["Synapse"]      =   &createInstance<SynapseC>;
    map["Determ"]       =   &createInstance<DetermC>;
    map["ExpHennequin"] =   &createInstance<ExpHennequinC>;
    map["OptimalStdp"]  =   &createInstance<OptimalStdpC>;
    map["SigmaTCLayer"]  =  &createInstance<SigmaTCLayerC>;

    return map;
}

