
#ifndef NEURONS_H
#define NEURONS_H

#include "core.h"
#include "simelem.h"

class Neurons : public SimElem {
    public:
        Neurons(int n, double a_v = 0.02, double b_v = 0.2, double c_v = -65, double d_v = 6, double V_rest = -70, double I0 = 0);
        void computeMe(double dt);
         
        double a;
        double b;
        double c;
        double d;
        double V_rest;
        vec u;
        vec Isyn;
        vec V;
};

#endif
