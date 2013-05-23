
#include "simelem.h"

#define I_OUT 14 // pA
#define LONG_S 0.01

class PoissonElem : public SimElem {
    PoissonElem(double herz_v = 0.05) : herz(herz_v) {}
    void computeMe(double dt) { 
       if(acc >= herz) {
            Iout=I_OUT;
       }
    }
    double herz;
    double Iout;

private:
    double acc;
};

