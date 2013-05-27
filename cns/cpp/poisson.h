#ifndef POISSONELEM_H
#define POISSONELEM_H

#include "basic_elements.h"
#include "core.h"

#define I_OUT 14 // pA
#define LONG_MS 10

class Poisson : public SimElem<vec, vec> {
public:
    Poisson(double mHerz_v);
    void computeMe(double dt);
    void setInput(vec in);
    vec getOutput();
    
    double mHerz;
    double Iout;
    
private:
    double acc;    
};

#endif
