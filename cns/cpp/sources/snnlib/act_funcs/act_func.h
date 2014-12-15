#pragma once

#include <snnlib/base.h>
#include <snnlib/config/constants.h>

class Neuron;

struct ActFuncRuntime {
    funDelegate prob;
    funDelegate probDeriv;
};


class ActFunc: public Obj  {
public:
	virtual void init(const ConstObj *_c, Neuron *n) = 0;
    virtual double prob(const double &u) const = 0;
    virtual double probDeriv(const double &u) const = 0;
    virtual void provideRuntime(ActFuncRuntime &rt) = 0;
protected:
	Neuron *n;
};
