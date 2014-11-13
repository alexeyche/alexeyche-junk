#pragma once


#include "act_func.h"

class Determ : public ActFunc {
public:
    Determ(DetermC &_c) : c(_c) {}

    double prob(const double &u) const { 
        if(u <= c.u_tr) return 0.0;    
        return 1.0;
    };
    
    double probDeriv(const double &u) {
        return 0.0;
    };
    
    const DetermC &c 
};


