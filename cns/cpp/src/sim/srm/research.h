#ifndef RESEARCH_H
#define RESEARCH_H

#include <sim/int/DEIntegrator.h>


#include "neurons.h"

namespace srm {
    class TSrmGrad {
    public:
        TSrmGrad(SrmNeuron *n_v) : n(n_v) {} 
        double p_stroke(double &t) {
            return SrmNeuron::beta/( 1 + exp(SrmNeuron::alpha*(SrmNeuron::tresh - n->u(t))) );
        }
        SrmNeuron *n;
    };

    double prob(const double &t, SrmNeuron *n) {
       return n->p(t);
    }
    
    double survFunction(SrmNeuron *n, double T0, double Tmax) {
        double t_left=T0;
        double p = 1;
//        Log::Info << "Surv.function of " << n->id() << "\n";
//        Log::Info << t_left << "|      ";
      for(size_t yi=0; yi<n->y.size(); yi++) {
          double t_right = n->y[yi];
          double no_spike = exp(-DEIntegrator<double>::Integrate(n, &prob, t_left, t_right-0.01, 1e-03));
          double spike = 1-exp(-prob(t_right, n));
//            Log::Info << "p:" << no_spike << "     |spike t:" << t_right << " p:" << spike << "|     ";
          p = p*no_spike*spike;
          t_left = t_right+0.01;
      }
      double no_spike = exp(-DEIntegrator<double>::Integrate(n, &prob, t_left, Tmax, 1e-03));
      p = p*no_spike;
//        Log::Info << no_spike << "|" << Tmax << "\n";
//        Log::Info << "p = " << p << "\n";
        return p;
    }
    void survFunctionSeq(SrmNeuron *n, double T0, double Tmax, double *out) {
        double t_left=T0;
        double p = 1;
        out[0] = exp(-DEIntegrator<double>::Integrate(n, &prob, T0, Tmax, 1e-03));
        for(size_t yi=0; yi<n->y.size(); yi++) {
            double t_right = n->y[yi];
            double no_spike = exp(-DEIntegrator<double>::Integrate(n, &prob, t_left, t_right-0.01, 1e-03));
            double spike = 1-exp(-prob(t_right, n));
            p = p*no_spike*spike;
            t_left = t_right+0.01;
            out[yi+1] = p*exp(-DEIntegrator<double>::Integrate(n, &prob, t_left, Tmax, 1e-03));
        }
        return;
    }   
};

#endif
