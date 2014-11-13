#pragma once

#include "learning_rule.h"


class Factory;

class OptimalStdp : public LearningRule {
protected:
    OptimalStdp() {}
    friend class Factory;
public:
    OptimalStdp(const OptimalStdpC *_c) : c(_c) {}


    shared_ptr<const OptimalStdpC> c;    
};

