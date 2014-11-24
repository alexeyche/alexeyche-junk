#pragma once

#include "learning_rule.h"

#include <snnlib/layers/common.h>
#include <snnlib/layers/neuron.h>

class Factory;

class OptimalStdp : public LearningRule {
protected:
    OptimalStdp() {}
    friend class Factory;

public:
    OptimalStdp(const OptimalStdpC *_c, Neuron *_n) {
    	init(_c, _n);
    }
    void init(const ConstObj *_c, Neuron *_n) {
        CAST_TYPE(OptimalStdpC, _c)
        c = cast;
        n = _n;
        n->setLearningRule(this);
    }

    void calculateWeightsDynamics()  {

    }
    Serializable* getStat() { return nullptr; }

    const OptimalStdpC *c;
    Neuron *n;
};

