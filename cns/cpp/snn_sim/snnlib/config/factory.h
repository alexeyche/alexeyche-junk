#pragma once

#include <snnlib/config/constants.h>
#include <snnlib/layers/layer.h>


typedef map<string, LayerObj*(*)()> layer_map_type;
typedef map<string, ConstObj*(*)()> const_map_type;

class Factory {
public:
    template<typename E,typename T> static E* createInstance() { return new T; }
    Factory() {
        const_map["IaFLayer"]     =   &createInstance<ConstObj, IaFLayerC>;
        const_map["Synapse"]      =   &createInstance<ConstObj, SynapseC>;
        const_map["Determ"]       =   &createInstance<ConstObj, DetermC>;
        const_map["ExpHennequin"] =   &createInstance<ConstObj, ExpHennequinC>;
        const_map["OptimalStdp"]  =   &createInstance<ConstObj, OptimalStdpC>;
        const_map["SigmaTCLayer"]  =  &createInstance<ConstObj, SigmaTCLayerC>;

    }

    ConstObj* createConstObj(string name, JsonBox::Value v) {
        string base_struct_name(name);
        auto it = const_map.find(name);
        if(it == const_map.end()) { \
            base_struct_name = Factory::findBaseStructName<const_map_type>(const_map, name);
        }
        ConstObj *o = const_map[base_struct_name]();        
        o->fill_structure(v);
        return o;
    }
    LayerObj* createLayerObj(string name, ConstObj *co) { 
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
    const_map_type const_map;
};


static Factory factory;
