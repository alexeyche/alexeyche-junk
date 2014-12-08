#pragma once


#include "act_func.h"

#include <snnlib/util/fastapprox/fastlog.h>
#include <snnlib/util/fastapprox/fastexp.h>

class Factory;

class ExpHennequin: public ActFunc {
protected:
    ExpHennequin() {}
    friend class Factory;
public:
    ExpHennequin(const ConstObj *_c, Neuron *_n) {
        init(_c, _n);
    }
    void init(const ConstObj *_c, Neuron *_n) {
        c = castType<ExpHennequinC>(_c);
        n = _n;
        n->setActFunc(this);
    }
    double prob(const double &u) const {
        double p = (c->p_rest + c->r0 * fastlog(1 + fastexp( c->beta*(u - c->u_tr) )))/1000.0;
        if(p>1.0) return 1.0;
        return p;
    };
    double probReversed(const double &p) const {
        //double u = log(exp((p/1000.0 - c->p_rest)/c->r0) - 1) ;
        //return u;
        return 0.0;
    }

    double probDeriv(const double &u) const {
        double exp_part = c->beta*(u - c->u_tr);
        double part;
        if(exp_part > 10.0) {
            part = 22026.47;
        } else {
            part = fastexp(exp_part);
        }
        return ( (part*c->r0*c->beta) / ( 1.0+part))/1000.0;
    };

    const ExpHennequinC *c;
};



