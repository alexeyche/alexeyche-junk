#ifndef POISSONELEM_H
#define POISSONELEM_H

#include "simelem.h"

#define I_OUT 14 // pA
#define LONG_S 0.01

class PoissonElem : public SimElem {
    PoissonElem(double herz_v = 0.05);
    void computeMe(double dt);
    double herz;
    double Iout;

private:
    double acc;
};

#endif
