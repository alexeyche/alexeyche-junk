#pragma once

#include "learning_rule.h"

#include <snnlib/layers/common.h>

class Factory;

class OptimalStdp : public LearningRule {
protected:
    OptimalStdp() {}
    friend class Factory;
public:
    OptimalStdp(const OptimalStdpC *_c) {
    	init(_c);
    }
    void init(const ConstObj *_c) {
        CAST_TYPE(OptimalStdpC, _c)
        c = shared_ptr<const OptimalStdpC>(cast);
    }

    shared_ptr<const OptimalStdpC> c;    
};

