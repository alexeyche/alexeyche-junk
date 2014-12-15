#pragma once

#include <snnlib/base.h>
#include <snnlib/util/json/json_box.h>

typedef map<string, Obj*(*)()> entity_map_type;
typedef map<string, Obj*(*)(string)> const_map_type;


class ConstObj;
class Layer;
class Synapse;
class ActFunc;
class LearningRule;
class TuningCurve;
class Neuron;
class NeuronConf;
class Constants;
class RuntimeGlobals;
class SerializableBase;
class RewardModulation;
class WeightNormalization;

class Factory {
    Factory();
    ~Factory() {
        cleanAll();
    }
    friend class ProtoRw;
public:
    template<typename BASE,typename INST> static BASE* createConstInstance(string name) { return new INST(name); }
    template<typename BASE,typename INST> static BASE* createInstance() { return new INST; }
    ConstObj *createConst(string name, JsonBox::Value v);
    ActFunc *createActFunc(string name, const Constants &c, Neuron *n);
    Synapse *createSynapse(string name, const Constants &c, size_t id_pre, double w, double dendrite_delay);
    Layer *createLayer(size_t size, bool wta, const NeuronConf &nc, const Constants &glob_c, RuntimeGlobals *run_glob_c);
    RewardModulation* createRewardModulation(string name, const Constants &c, Neuron *n, RuntimeGlobals *run_glob_c);
    Neuron *createNeuron(string name, size_t local_id, const Constants &c, const RuntimeGlobals *run_glob_c, double axon_delay);
    TuningCurve *createTuningCurve(string name, const Constants &c,  size_t layer_size, size_t neuron_id, Neuron *n);
    LearningRule * createLearningRule(string name, const Constants &c, Neuron *n, WeightNormalization *wnorm);
    SerializableBase* createSerializable(const string &name);
    WeightNormalization* createWeightNormalization(string name, const Constants &c, Neuron *n);

    template <typename T>
    T* registerObj(T *o) {
        objects.push_back(o);
        return o;
    }
    void cleanAllDynamicObj() {
        auto it=dyn_objects.begin();
        while(it != dyn_objects.end()) {
            Obj *o_in = *it;
            dyn_objects.erase(it);
            delete o_in;
        }
    }
    void cleanAll() {
        for(auto it=objects.begin(); it != objects.end(); ++it) {
            delete *it;
        }
        for(auto it=dyn_objects.begin(); it != dyn_objects.end(); ++it) {
            delete *it;
        }
 
        objects.clear();
        dyn_objects.clear();
    }

    void cleanObj(Obj *o) {
        auto it=objects.begin();
        while(it != objects.end()) {
            Obj *o_in = *it;
            if(o == o_in) {
                objects.erase(it);
                delete o_in;
            } else {
                ++it;
            }
        }
    }

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
    const_map_type const_map;
    static Factory *_inst;
    vector<Obj*> objects;
    vector<Obj*> dyn_objects;
};


