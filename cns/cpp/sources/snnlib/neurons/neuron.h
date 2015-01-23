#pragma once

#include <snnlib/base.h>
#include <snnlib/act_funcs/act_func.h>
#include <snnlib/learning/learning_rule.h>
#include <snnlib/config/constants.h>
#include <snnlib/tuning_curves/tuning_curve.h>
#include <snnlib/sim/runtime_globals.h>
#include <snnlib/serialize/proto_rw.h>
#include <snnlib/config/factory.h>
#include <snnlib/serialize/serialize.h>
#include <snnlib/reinforcement/reward_modulation.h>

#include "synapse.h"
#include "neuron_stat.h"

#include <snnlib/protos/model.pb.h>
//#include "neuron_model.h"

extern size_t global_neuron_index;


class ProtoRw;

#define STAT_COLLECT_LIMIT 10000

#define SYN_ACT_TOL 0.0001

struct NeuronRuntime {
    stateDelegate calculateProbability;
    stateDelegate calculateDynamics;
    propSynSpikeDelegate propagateSynSpike;
    attachDelegate attachCurrent;
};


class Neuron: public Serializable<Protos::Neuron> {
protected:
    Neuron() : Serializable(ENeuron) {}
    friend class Factory;
public:
    Neuron(const ConstObj *_c, size_t _local_id, const RuntimeGlobals *_glob_c, double _axon_delay);
    Neuron(const Neuron &another);

    // init
    virtual void init(const ConstObj *_c, size_t _local_id, const RuntimeGlobals *_glob_c, double _axon_delay);
    void setActFunc(ActFunc *_act);
    void setLearningRule(LearningRule *_lrule);
    void setTuningCurve(TuningCurve *_tc);
    void setRewardModulation(RewardModulation *_rmod);
    bool hasConnection(const size_t &id) {
        for(auto it=syns.begin(); it != syns.end(); ++it) {
            if((*it)->id_pre == id) return true;
        }
        return false;
    }
    inline const Reward* getReward() {
        return glob_c->getReward(id);
    }
    void addSynapse(Synapse *s);
    void addSynapseAtAllocatedPos(Synapse *s, const size_t &pos_i);
    // runtime
    virtual void calculateProbability() = 0;
    virtual void calculateDynamics() = 0;
    virtual void attachCurrent(const double &I) = 0;
    virtual void propagateSynSpike(const SynSpike *sp) = 0;
    virtual void reset();
    virtual void provideRuntime(NeuronRuntime &rt) = 0;
    // stat funcs
    virtual void saveStat(SerialPack &p);
    virtual void enableCollectStatistics();
    void enableCollectProbStatistics();
    // serialize
    void deserialize();
    ProtoPack serialize();
    void saveModel(ProtoRw &rw);
    void loadModel(ProtoRw &rw);

    void print(std::ostream& str) const;

    size_t id;
    size_t local_id;

    double y;
    double p;
    double M;

    double gr;
    double ga;
    uchar fired;

    double axon_delay;

    vector<Synapse*> syns;
    list<size_t> active_synapses;

    // run time interfaces
    ActFuncRuntime act_rt;
    LearningRuleRuntime lrule_rt;
    RewardModulationRuntime rmod_rt;
    TuningCurveRuntime tc_rt;
    //vector<SynapseRuntime> syns_rt;

    const RuntimeGlobals *glob_c;
protected:
    NeuronStat *stat;

    const ConstObj *bc;

    LearningRule *lrule;
    TuningCurve *tc;
    RewardModulation *rmod;

    bool collectStatistics;
};



