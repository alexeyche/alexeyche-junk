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
            (*it)->w += c->learning_rate * dw;
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

