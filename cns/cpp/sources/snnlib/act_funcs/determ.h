#pragma once


#include "act_func.h"

class Factory;

class Determ : public ActFunc {
protected:
    Determ() {}
    friend class Factory;
public:
    Determ(const DetermC *_c) {
        init(_c);
    }
    void init(const ConstObj *_c) {
        CAST_TYPE(DetermC, _c)
        c = shared_ptr<const DetermC>(cast);
    }

    double prob(const double &u) const { 
        if(u <= c->u_tr) return 0.0;    
        return 1.0;
    };
    
    double probDeriv(const double &u) const {
        return 0.0;
    };
    
    shared_ptr<const DetermC> c;
};


