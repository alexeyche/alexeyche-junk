#pragma once

#include "learning_rule.h"

#include <snnlib/neurons/neuron.h>

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
        c = castType<OptimalStdpC>(_c);
        n = _n;
    }

    void calculateWeightsDynamics()  {

    }
    void saveStat(SerialFamily &p) {  }
    void saveModel(SerialFamily &p) {  }

    const OptimalStdpC *c;
    Neuron *n;
};

