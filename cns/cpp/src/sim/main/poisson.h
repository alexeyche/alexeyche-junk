#ifndef POISSONELEM_H
#define POISSONELEM_H

#include "basic_elements.h"

#define I_OUT 14 // pA
#define LONG_MS 100

class Poisson : public SimElem<vec, vec> {
public:
    Poisson(double mHerz, double long_ms, double Iout_value);
    void computeMe(double dt);
    
    void setInput(vec in);
    vec getOutput();
    
    double mHerz;
    double Iout;
    double Iout_value;
    double long_ms;
    
private:
    double acc;    
};

#endif
