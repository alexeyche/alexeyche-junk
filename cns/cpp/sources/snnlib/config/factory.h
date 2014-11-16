#pragma once

#include <snnlib/base.h>
#include <snnlib/util/json/json_box.h>

typedef map<string, Obj*(*)()> entity_map_type;


class ConstObj;
class Layer;
class Synapse;
class ActFunc;
class LearningRule;
class TuningCurve;

class Factory {
    Factory();
public:
    template<typename BASE,typename INST> static BASE* createInstance() { return new INST; }
    ConstObj* createConst(string name, JsonBox::Value v);

    Layer* createLayer(string name, size_t id, size_t size, const ConstObj *c, const ActFunc *act, const LearningRule *lrule);
    Layer* createInputLayer(string name, size_t id, size_t size, const ConstObj *c, const ActFunc *act, const TuningCurve *tc);  
    Synapse*  createSynapse(string name, const ConstObj *c, size_t id_pre, double w);
    TuningCurve* createTuningCurve(string name, size_t size, const ConstObj *c);
    ActFunc* createActFunc(string name, const ConstObj *c);
    LearningRule*  createLearningRule(string name, const ConstObj *c);

    string findBaseStructName(string deriv_struct_name) {
        for(auto it=entity_map.begin(); it != entity_map.end(); ++it) {
            string base_struct_name = it->first;
            if(deriv_struct_name.substr(0, base_struct_name.size()) == base_struct_name) {
                return base_struct_name;
            }
        }
        for(auto it=const_map.begin(); it != const_map.end(); ++it) {
            string base_struct_name = it->first;
            if(deriv_struct_name.substr(0, base_struct_name.size()) == base_struct_name) {
                return base_struct_name;
            }
        }
        cerr << "Unrecognized typename: " << deriv_struct_name << "\n";
        terminate();
    }
    static Factory* inst();
private:
    entity_map_type entity_map;
    entity_map_type const_map;
    static Factory *_inst;
};


