#pragma once

#include <snnlib/base.h>
#include <snnlib/util/json/json_box.h>
#include <snnlib/util/distributions.h>

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
    Layer *createLayer(size_t size, bool wta, const NeuronConf &nc, const Constants &glob_c, RuntimeGlobals *run_glob_c, bool learning);
    RewardModulation* createRewardModulation(string name, const Constants &c, Neuron *n, RuntimeGlobals *run_glob_c);
    Neuron *createNeuron(string name, size_t local_id, const Constants &c, const RuntimeGlobals *run_glob_c, double axon_delay);
    TuningCurve *createTuningCurve(string name, const Constants &c,  size_t layer_size, size_t neuron_id, Neuron *n);
    LearningRule * createLearningRule(string name, const Constants &c, Neuron *n, ActFunc *act_f, WeightNormalization *wnorm);
    SerializableBase* createSerializable(const string &name);
    WeightNormalization* createWeightNormalization(string name, const Constants &c, Neuron *n);

    template <typename T>
    Distribution<T> *createDistribution(const string &str_init) {
        Distribution<T> *o;
        if(strStartsWith(str_init, "Exp")) {
            vector<double> params = parseParenthesis(str_init);
            if(params.size() != 2) {
                cerr << "Bad parameters to Exp distribution: " << str_init << "\n";
                terminate();
            }
            o = new ExpDistribution(params[0], params[1]);
        } else
        if(strStartsWith(str_init, "Norm")) {
            vector<double> params = parseParenthesis(str_init);
            if(params.size() != 2) {
                cerr << "Bad parameters to Norm distribution: " << str_init << "\n";
                terminate();
            }
            o = new NormalDistribution(params[0], params[1]);
        } else
        if(strStartsWith(str_init, "Unif")) {
            vector<double> params = parseParenthesis(str_init);
            if(params.size() != 2) {
                cerr << "Bad parameters to Unif distribution: " << str_init << "\n";
                terminate();
            }
            o = new UniformDistribution(params[0], params[1]);
        } else {
            cerr << "Unknown Distribution " << str_init << "\n";
            terminate();
        }
        objects.push_back(o);
        return o;
    }

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
        vector<string> del0 = split(deriv_struct_name, '_');
        assert(del0.size() > 0);
        string base_struct_name = del0[0];
        if(entity_map.find(base_struct_name) != entity_map.end()) {
            return base_struct_name;
        }
        if(const_map.find(base_struct_name) != const_map.end()) {
            return base_struct_name;
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


