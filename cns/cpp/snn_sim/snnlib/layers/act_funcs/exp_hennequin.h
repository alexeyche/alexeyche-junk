#pragma once


#include "act_func.h"

class Factory;

class ExpHennequin: public ActFunc {
protected:
    ExpHennequin() {}
    friend class Factory;
public:
    ExpHennequin(const ExpHennequinC *_c) : c(_c) {}

    double prob(const double &u) const { 
        if(u <= c->u_tr) return 0.0;    
        return 1.0;
    };
    
    double probDeriv(const double &u) const {
        return 0.0;
    };
    
    shared_ptr<const ExpHennequinC> c; 
};


