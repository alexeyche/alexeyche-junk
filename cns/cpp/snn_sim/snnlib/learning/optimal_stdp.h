#pragma once

#include "learning_rule.h"


class OptimalStdp : public LearningRule {
public:
    OptimalStdp(OptimalStdpC &_c) : c(_c) {}


    const OptimalStdpC &c;    
};

