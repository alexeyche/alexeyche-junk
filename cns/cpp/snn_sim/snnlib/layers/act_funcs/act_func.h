#pragma once

#include <snnlib/base.h>

class ActFunc: public Obj  {
public:
    virtual double prob(const double &u) const = 0;        
    virtual double probDeriv(const double &u) const = 0;        
};
