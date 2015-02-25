#pragma once

#include "act_function.h"




struct DetermC : public Constants {
    double treshold;
};


class Determ : public ActFunction<DetermC> {
public:
    double prob(const double &u) {
        if(u >= c.treshold) {
            return 1.0;
        }
        return 0.0;
    }
    double probDeriv(const double &u) {
        return 0.0;
    }

    void provideInterface(ActFunctionInterface &i) {
        i.prob = MakeDelegate(this, &Determ::prob);
        i.probDeriv = MakeDelegate(this, &Determ::probDeriv);
    }

};
