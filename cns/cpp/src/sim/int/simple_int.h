#ifndef SIMPLE_INT
#define SIMPLE_INT

#include <sim/srm/neurons.h>

template <typename T>
double int_trapezium(double from, double to, double n, T *inst, double (*f)(const double&, T*))
{
   double h = (to - from) / n;
   double sum = (*f)(from, inst) + (*f)(to, inst);
   int i;
   for(i = 1;i < n;i++)
       sum += 2.0*(*f)(from + i * h, inst);
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
