#pragma once

#include <snnlib/base.h>

static size_t global_neuron_index = 0;

#include "synapse.h"

#include <snnlib/act_funcs/act_func.h>
#include <snnlib/learning/learning_rule.h>
#include <snnlib/config/constants.h>
#include <snnlib/tuning_curves/tuning_curve.h>


#define SYN_ACT_TOL 0.0001

class Neuron: public Printable {
protected:
    Neuron() {}
    friend class Factory;
public:
    Neuron(const ConstObj *_c, ActFunc *_act, LearningRule *_lrule, TuningCurve *_tc) {
        init(_c, _act, _lrule, _tc);
    }
    virtual void init(const ConstObj *_c, ActFunc *_act, LearningRule *_lrule, TuningCurve *_tc) {
        id = ++global_neuron_index;
        bc = _c;
        act = _act;
        lrule = _lrule;
        tc = _tc;

        y = 0.0;
        p = 0.0;
        fired = 0;
    }

    size_t id;

    double y;
    double p;
    uchar fired;

    vector<Synapse*> syns;

    void addSynapse(Synapse *s) {
        syns.push_back(s);
    }
    virtual void calculateProbability() = 0;
    virtual void calculateDynamics() = 0;
    virtual void attachCurrent(const double &I) = 0;



    void print(std::ostream& str) const {
        str << "Neuron(" << id << ")\n";
        str << "\ty == " << y;
        str << "\tsynapses: \n";
        for(auto it=syns.begin(); it != syns.end(); ++it) {
            Synapse *s = *it;
            str << *s << ", ";
        }
        str << "\n";
    }
protected:
    list< Synapse *> active_synapses;

    const ConstObj *bc;
    ActFunc *act;
    LearningRule *lrule;
    TuningCurve *tc;
};


