
#include "neurons.h"

Neurons::Neurons(int n, double a_v, double b_v, double c_v, double d_v, double V_rest_v, double I0) :  SimElem(n,n), u(n), Isyn(n), V(n)
{
    a = a_v;
    b = b_v;
    c = c_v;
    d = d_v;
    
    V_rest = V_rest_v;
    V.fill(V_rest);
    u.fill(V_rest * b);
    Isyn.fill(I0);
}

void Neurons::computeMe(double dt) {
    uvec need_reset = find(V>30);    
    V = V + dt * (0.04*square(V) + 5*V - V_rest*2 - u + Isyn);
    u = u + dt * a * (b*V - u);
}

void Neurons::setInput(vec I) {    
    Isyn = I;
}

vec Neurons::getOutput() {
    return V;
}
