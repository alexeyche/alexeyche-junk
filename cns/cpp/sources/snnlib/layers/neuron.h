#pragma once

#include <snnlib/base.h>

static size_t global_neuron_index = 0;

#include "synapse.h"

#include <snnlib/act_funcs/act_func.h>
#include <snnlib/learning/learning_rule.h>
#include <snnlib/config/constants.h>
#include <snnlib/tuning_curves/tuning_curve.h>


class Neuron: public Printable {
public:
    Neuron(size_t _id, const ConstObj *_c, const ActFunc *_act, const LearningRule *_lrule, const TuningCurve *_tc) {
        id = _id;
        bc = shared_ptr<const ConstObj>(_c);
        act = shared_ptr<const ActFunc>(_act);
        lrule = shared_ptr<const LearningRule>(_lrule);
        tc = shared_ptr<const TuningCurve>(_tc);

        y = 0.0;
        p = 0.0;
        fired = 0;
    }
    
    size_t id;
    
    double y;
    double p;
    uchar fired;

    vector<Synapse> syns;
    
    void addSynapse(Synapse s) {
        syns.push_back(s);
    }
    virtual void calculateProbability() = 0;
    virtual void calculateDynamics() = 0;
    
    void print(std::ostream& str) const {
        str << "Neuron(" << id << ")\n";
        str << "\ty == " << y;
        str << "\tsynapses: \n";
        for(auto it=syns.begin(); it != syns.end(); ++it) {
            str << *it << ", ";
        }
        str << "\n";
    }
protected:    
    shared_ptr<const ConstObj> bc;
    shared_ptr<const ActFunc> act;
    shared_ptr<const LearningRule> lrule;
    shared_ptr<const TuningCurve> tc;
};


