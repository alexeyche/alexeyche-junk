#pragma once

#include <snnlib/base.h>

static size_t global_neuron_index = 0;

#include "synapse.h"

#include <snnlib/act_funcs/act_func.h>
#include <snnlib/learning/learning_rule.h>
#include <snnlib/config/constants.h>
#include <snnlib/tuning_curves/tuning_curve.h>

#include <snnlib/sim/runtime_globals.h>

#include <snnlib/serialize/serialize.h>

#include "neuron_stat.h"

#define SYN_ACT_TOL 0.0001

class Neuron: public Printable {
protected:
    Neuron() {}
    friend class Factory;
public:
    Neuron(const ConstObj *_c, const RuntimeGlobals *_glob_c, double _axon_delay) {
        init(_c, _glob_c, _axon_delay);
    }
    ~Neuron() {
        if(stat) {
            delete stat;
        }
    }
    virtual void init(const ConstObj *_c, const RuntimeGlobals *_glob_c, double _axon_delay) {
        id = global_neuron_index++;
        bc = _c;
        glob_c = _glob_c;

        act = nullptr; lrule = nullptr; tc = nullptr;

        y = 0.0;
        p = 0.0;
        fired = 0;

        collectStatistics = false;
        stat = nullptr;
        axon_delay = _axon_delay;
    }
    void setActFunc(ActFunc *_act) {
        act = _act;
    }
    void setLearningRule(LearningRule *_lrule) {
        lrule = _lrule;
    }
    void setTuningCurve(TuningCurve *_tc) {
        tc = _tc;
    }
    size_t id;

    double y;
    double p;
    uchar fired;
    double axon_delay;

    vector<Synapse*> syns;

    void addSynapse(Synapse *s) {
        syns.push_back(s);
        if(collectStatistics) {
            stat->syns.push_back(vector<double>());
        }
    }

    //runtime
    virtual void calculateProbability() = 0;
    virtual void calculateDynamics() = 0;
    virtual void attachCurrent(const double &I) = 0;
    virtual void propagateSynSpike(const SynSpike *sp) = 0;
    virtual void provideDelegates(RunTimeDelegates &rtd) {}

    // stat funcs
    virtual Serializable* getStat() {
        return stat;
    }
    virtual void enableCollectStatistics() {
        collectStatistics = true;
        stat = new NeuronStat(this);
    }

    //virtual vector<string> getDependentConstantsNames() { return vector<string>(); }
    //virtual void setDependentConstants(const vector<const ConstObj*> &constants) { }

    void print(std::ostream& str) const {
        str << "Neuron(" << id << ")\n";
        str << "\ty == " << y << ", axon_delay: " << axon_delay << ", synapses\n";
        for(auto it=syns.begin(); it != syns.end(); ++it) {
            Synapse *s = *it;
            str << *s << ", ";
        }
        str << "\n";
    }

    NeuronStat *stat;
protected:
    list< Synapse *> active_synapses;

    const ConstObj *bc;
    const RuntimeGlobals *glob_c;

    ActFunc *act;
    LearningRule *lrule;
    TuningCurve *tc;

    bool collectStatistics;

};


