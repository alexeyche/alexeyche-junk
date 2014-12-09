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

static size_t global_neuron_index = 0;


class ProtoRw;

#define STAT_COLLECT_LIMIT 10000

#define SYN_ACT_TOL 0.0001

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

    inline const Reward* getReward() {
        return glob_c->getReward(id);
    }
    void addSynapse(Synapse *s);

    // runtime
    virtual void calculateProbability() = 0;
    virtual void calculateDynamics() = 0;
    virtual void attachCurrent(const double &I) = 0;
    virtual void propagateSynSpike(const SynSpike *sp) = 0;
    virtual void provideDelegates(RunTimeDelegates &rtd) {}
    virtual void reset();

    // stat funcs
    virtual void saveStat(SerialPack &p);
    virtual void enableCollectStatistics();
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
    double weight_factor;
    double fired;

    double axon_delay;

    vector<Synapse*> syns;
    list<size_t> active_synapses;

    ActFunc *act;
    const RuntimeGlobals *glob_c;
protected:
    NeuronStat *stat;

    const ConstObj *bc;



    LearningRule *lrule;
    TuningCurve *tc;
    RewardModulation *rmod;

    bool collectStatistics;
};



