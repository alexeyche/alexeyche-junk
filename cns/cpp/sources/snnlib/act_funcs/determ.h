#pragma once


#include "act_func.h"

class Factory;

class Determ : public ActFunc {
protected:
    Determ() {}
    friend class Factory;
public:
    Determ(const ConstObj *_c, Neuron *_n) {
        init(_c, _n);
    }
    void init(const ConstObj *_c, Neuron *_n) {
        CAST_TYPE(DetermC, _c)
        c = cast;
        n = _n;
        n->setActFunc(this);
    }

    double prob(const double &u) const {
        if(u <= c->u_tr) return 0.0;
        return 1.0;
    };

    double probDeriv(const double &u) const {
        return 0.0;
    };

    const DetermC *c;
};


