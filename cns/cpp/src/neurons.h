
#ifndef NEURONS_H
#define NEURONS_H


#include "basic_elements.h"

class Neurons : public SimElem<vec, vec> {
    public:
        Neurons(int n, double a_v = 0.02, double b_v = 0.2, double c_v = -65, 
                double d_v = 6, double V_rest = -70, double I0 = 0, double treshold_c = 30);
        void computeMe(double dt);  
        
        void setInput(vec I);
        vec getOutput();

        int n;

        double a;
        double b;
        double c;
        double d;
        double V_rest;
        double treshold;        
        vec u;
        vec Isyn;
        vec V;
        vec Vout;
    private:
        uvec need_reset;        
};

#endif
