#ifndef SIMPLE_INT
#define SIMPLE_INT

#include <sim/srm/neurons.h>

double int_trapezium(double from, double to, double n, srm::SrmNeuron &inst, double (srm::SrmNeuron::*f)(const double&))
{
   double h = (to - from) / n;
   double sum = (inst.*f)(from) + (inst.*f)(to);
   int i;
   for(i = 1;i < n;i++)
       sum += 2.0*(inst.*f)(from + i * h);
   return  h * sum / 2.0;
}

double int_brute(double from, double to, double dt, srm::SrmNeuron &inst, double (srm::SrmNeuron::*f)(const double&)) {
    vec t = linspace<vec>(from, to, (int)(to-from)/dt);
    double intv=0;
    for(size_t ti=0; ti<t.n_elem; ti++) {
        intv += (inst.*f)(t[ti])*dt;    
    }
    return intv;
}


#endif
