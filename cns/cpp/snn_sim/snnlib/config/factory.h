#pragma once

#include <snnlib/config/constants.h>
#include <snnlib/layers/layer.h>
#include <snnlib/learning/optimal_stdp.h>

#include <typeinfo>

typedef map<string, Entity*(*)()> entity_map_type;

template<typename E,typename T> static E* createInstance() { return new T; }


class Factory {
public:
    Factory() {
        entity_map["SRMLayer"]  = &createInstance<Entity, SRMLayer<SRMNeuron>>;
        entity_map["Synapse"]      =   &createInstance<Entity, SynapseC>;
        entity_map["Determ"]       =   &createInstance<Entity, DetermC>;
        entity_map["ExpHennequin"] =   &createInstance<Entity, ExpHennequinC>;
        entity_map["OptimalStdp"]  =   &createInstance<Entity, OptimalStdpC>;
        entity_map["SigmaTCLayer"]  =  &createInstance<Entity, SigmaTCLayerC>;
    }


    ConstObj* createConstObj(string name, JsonBox::Value v) {
        string base_struct_name(name);
        auto it = entity_map.find(name);
        if(it == entity_map.end()) { \
            base_struct_name = Factory::findBaseStructName<entity_map_type>(entity_map, name);
        }
        ConstObj *o = static_cast<ConstObj*>(entity_map[base_struct_name]());
        o->fill_structure(v);
        return o;
    }

    LayerObj* createLayerObj(string name, size_t id, size_t size, Obj &act, Obj &lrule) { 
    }

    template <typename T> static string findBaseStructName(T &tmap, string deriv_struct_name) {
        for(auto it=tmap.begin(); it != tmap.end(); ++it) {
            string base_struct_name = it->first;
            if(deriv_struct_name.substr(0, base_struct_name.size()) == base_struct_name) {
                return base_struct_name;
            }
        }
        cerr << "Unrecognized typename: " << deriv_struct_name << "\n";
        terminate();
    }
private:
    entity_map_type entity_map;
};


static Factory factory;
