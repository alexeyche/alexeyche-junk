#pragma once

#include <snnlib/base.h>
#include <snnlib/act_funcs/act_func.h>
#include <snnlib/learning/learning_rule.h>
#include <snnlib/config/constants.h>
#include <snnlib/tuning_curves/tuning_curve.h>
#include <snnlib/sim/runtime_globals.h>
#include <snnlib/serialize/proto_rw.h>
#include <snnlib/config/factory.h>

#include "synapse.h"
#include "neuron_stat.h"

#include <snnlib/protos/model.pb.h>
//#include "neuron_model.h"

static size_t global_neuron_index = 0;


class ProtoRw;

#define STAT_COLLECT_LIMIT 10000

#define SYN_ACT_TOL 0.0001

class Neuron: public SerializableModel {
protected:
    Neuron() {}
    friend class Factory;
public:
    Neuron(const ConstObj *_c, const RuntimeGlobals *_glob_c, double _axon_delay);
    Neuron(const Neuron &another);

    // init
    virtual void init(const ConstObj *_c, const RuntimeGlobals *_glob_c, double _axon_delay);
    void setActFunc(ActFunc *_act);
    void setLearningRule(LearningRule *_lrule);
    void setTuningCurve(TuningCurve *_tc);
    void addSynapse(Synapse *s);

    void loadModel(ProtoRw &rw);
    void saveModel(ProtoRw &rw);
    // runtime
    virtual void calculateProbability() = 0;
    virtual void calculateDynamics() = 0;
    virtual void attachCurrent(const double &I) = 0;
    virtual void propagateSynSpike(const SynSpike *sp) = 0;
    virtual void provideDelegates(RunTimeDelegates &rtd) {}

    // stat funcs
    virtual void saveStat(SerialPack &p);
    virtual void enableCollectStatistics();

    void print(std::ostream& str) const;

    size_t id;

    double y;
    double p;
    uchar fired;

    double axon_delay;

    vector<Synapse*> syns;
protected:
    NeuronStat *stat;

    list< Synapse *> active_synapses;

    const ConstObj *bc;
    const RuntimeGlobals *glob_c;

    ActFunc *act;
    LearningRule *lrule;
    TuningCurve *tc;

    bool collectStatistics;
};



