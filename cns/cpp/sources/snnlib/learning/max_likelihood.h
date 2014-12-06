#pragma once

#include "learning_rule.h"

#include <snnlib/neurons/neuron.h>
#include <snnlib/learning/srm_methods.h>

class Factory;

class MaxLikelihood : public LearningRule  {
protected:
    MaxLikelihood() {
        Serializable::init(EMaxLikelihood);
    }
    friend class Factory;

public:
    MaxLikelihood(const MaxLikelihoodC *_c, Neuron *_n) {
        init(_c, _n);
    }
    void init(const ConstObj *_c, Neuron *_n) {
        c = castType<MaxLikelihoodC>(_c);
        n = _n;
        eligibility_trace = 0.0;

        Serializable::init(EMaxLikelihood);
    }

    void calculateWeightsDynamics()  {
        for(auto it=n->active_synapses.begin(); it != n->active_synapses.end(); ++it) {
            double dw = SRMMethods::dLLH_dw(n, *it);    
            if(std::isnan(dw)) {
                cout << "Found nan dw:\n";
                cout << *n;
                terminate();
            }
//            (*it)->w += c->learning_rate * dw;
            if(std::isnan((*it)->w)) {
                cout << "Found nan w:\n";
                cout << *n;
                terminate();
            }
        }
    }
    void deserialize() {
    }
    ProtoPack serialize() {
        return getNew();
    }
    ProtoPack getNew() {
        return ProtoPack();
    }

    void print(std::ostream& str) const { }

    double eligibility_trace;

    const MaxLikelihoodC *c;
    Neuron *n;
};

