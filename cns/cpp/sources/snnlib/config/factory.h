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
class Neuron;
class NeuronConf;
class Constants;

class Factory {
    Factory();
    ~Factory() {
        for(auto it=objects.begin(); it != objects.end(); ++it) {
            delete *it;
        }
        objects.clear();
    }
public:
    template<typename BASE,typename INST> static BASE* createInstance() { return new INST; }
    ConstObj *createConst(string name, JsonBox::Value v);
    ActFunc *createActFunc(string name, const Constants &c, Neuron *n);
    Synapse *createSynapse(string name, const Constants &c, size_t id_pre, double w, double dendrite_delay);
    Layer *createLayer(size_t size, const NeuronConf &nc, const Constants &glob_c);
    Neuron *createNeuron(string name, const Constants &c, double axon_delay);
    TuningCurve *createTuningCurve(string name, const Constants &c, Neuron *n);
    LearningRule * createLearningRule(string name, const Constants &c, Neuron *n);

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
    static Factory& inst();
private:
    entity_map_type entity_map;
    entity_map_type const_map;
    static Factory *_inst;
    vector<Obj*> objects;
};


