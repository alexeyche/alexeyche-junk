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
        CAST_TYPE(OptimalStdpC, _c)
        c = cast;
        n = _n;
    }

    void calculateWeightsDynamics()  {

    }
    void saveStat(SerialPack &p) {  }
    void saveModel(SerialPack &p) {  }
    
    const OptimalStdpC *c;
    Neuron *n;
};

