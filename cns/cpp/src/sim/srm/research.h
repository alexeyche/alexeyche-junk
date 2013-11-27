#ifndef RESEARCH_H
#define RESEARCH_H

#include <sim/int/DEIntegrator.h>
#include <sim/int/gauss_legendre.h>
#include <sim/int/simple_int.h>

#include "neurons.h"

namespace srm {

    double prob(const double &t, SrmNeuron *n) {
       return n->p(t);
    }
    
    double integrand_gl(double x, void* data) {
        srm::SrmNeuron *n = (srm::SrmNeuron*)data;
        return n->p(x);
    }
    
    double break_apart_int(srm::SrmNeuron *n, double T0, double Tmax, double dt) {
        double integral = 0;
        for(double t=T0; t<Tmax; t+=dt) {
            double t_left = t;
            double t_right = t+dt;
            if(t_right>Tmax) { t_right = Tmax; t=datum::inf; } 
                                 
        #if LOCAL_INT_METHOD==gauss
            double integral_cur = gauss_legendre(128, integrand_gl, (void*)n, t_left, t_right);
        #elif LOCAL_INT_METHOD==de
            double integral_cur = DEIntegrator<double, SrmNeuron*>::Integrate(n, &prob, t_left, t_right, 1e-04);
        #endif
        #if VERBOSE_SURV==1           
            Log::Info << "integral at " << t_left << ":" << t_right << " = " << integral_cur << "\n";
        #endif            
            integral += integral_cur;
        }
    #if VERBOSE_SURV==1           
        Log::Info << "integral at whole " << T0 << ":" << Tmax<< " = " << integral << "\n";
    #endif
        return integral;
    }   
    
    #define INT_NUM_BREAKS (Tmax-T0)/1.0
    double survFunction(SrmNeuron *n, double T0, double Tmax) {
        double p = exp( - break_apart_int(n, T0, Tmax, INT_NUM_BREAKS ));  // prob of no spikes at whole region
        for(int yi = n->y.n_elem(Tmax)-1; yi>=0; yi--) {
            double p_sp = n->p(n->y(yi)); // prob of spikes
        #if VERBOSE_SURV==1           
            Log::Info << "p_sp("<< n->y[yi] <<") = " << p_sp << "\n";
        #endif            
            p = p * p_sp;
        }
        return p;
    }


    double survFunction__old(SrmNeuron *n, double T0, double Tmax) {
        double t_right=Tmax;
        double p = 0;
        Log::Info << "Surv.function of " << n->id() << " (from behind)\n";
        Log::Info << t_right << "|      ";
        for(int yi = n->y.n_elem(Tmax)-1; yi>=0; yi--) {
            if(n->y[yi]<T0) { break; } 
            double t_left = n->y[yi];
//            double no_spike = exp(-DEIntegrator<double, SrmNeuron*>::Integrate(n, &prob, t_left+1e-12, t_right, 1e-06));
            double no_spike = 0;
            double tt;
            for(tt=t_left; tt<(t_right-1); tt+=1) {
                no_spike += int_brute<srm::SrmNeuron>(tt, tt+1, 0.00001, n, &prob);
                Log::Info << " ppp: " << no_spike << "  at " << tt << ":" << tt+1 << "\n";
            }
            no_spike += int_brute<srm::SrmNeuron>(tt, t_right, 0.00001, n, &prob);
                Log::Info << " ppp: " << no_spike << "  at " << tt << ":" << t_right << "\n";
            double spike = 1-exp(-prob(t_left, n));
            Log::Info << "p:" << no_spike << "     |spike t:" << t_left << " p:" << spike << "|     ";
            p = p + no_spike; //*spike;
            t_right = t_left-0.001;
        }
//        double no_spike = exp(-DEIntegrator<double, SrmNeuron*>::Integrate(n, &prob, T0, t_right, 1e-06));
        double no_spike=0;
            double tt;
            for(tt=T0; tt<(t_right-1); tt+=1) {
                no_spike += int_brute<srm::SrmNeuron>(tt, tt+1, 0.00001, n, &prob);
                Log::Info << " ppp: " << no_spike << "  at " << tt << ":" << tt+1 << "\n";
            }        
            no_spike += int_brute<srm::SrmNeuron>(tt, t_right, 0.00001, n, &prob);
                Log::Info << " ppp: " << no_spike << "  at " << tt << ":" << t_right << "\n";

        p = p +no_spike;
        Log::Info << no_spike << "|" << T0 << "\n";
        Log::Info << "p = " << p << "\n";
        return p;
    }
    
    double survFunction__test(SrmNeuron *n, double T0, double Tmax) {
//        double p = exp(-DEIntegrator<double, SrmNeuron*>::Integrate(n, &prob, T0, Tmax, 1e-06));  // no spike     
        double p = 0;
        for(double tt=T0; tt<Tmax; tt+=1) {
            p = p + int_brute<srm::SrmNeuron>(tt, tt+1, 0.00001, n, &prob); 
            Log::Info << " ppp: " << p << "  at " << tt << ":" << tt+1 << "\n";
        }            
        Log::Info << "p_no_spike["<< T0 << "," << Tmax << "]: " << p << "\n";
//        for(int yi = n->y.n_elem(Tmax)-1; yi>=0; yi--) {
//        for(size_t yi=0; yi<n->y.size(); yi++) {
//            double p_spike = 1-exp(-prob(n->y[yi], n));
//            Log::Info << "p_spike["<< n->y[yi] << "]: " << p_spike << " | ";
//            p = p*p_spike;
//        }
        Log::Info << "\n";
        return p;
    }

//    void survFunctionSeq__old(SrmNeuron *n, double T0, double Tmax, double *out) {
//        double t_left=T0;
//        double p = 1;
//        out[0] = exp(-DEIntegrator<double, SrmNeuron*>::Integrate(n, &prob, T0, Tmax, 1e-03));
//        for(size_t yi=0; yi<n->y.size(); yi++) {
//            double t_right = n->y[yi];
//            double no_spike = exp(-DEIntegrator<double, SrmNeuron*>::Integrate(n, &prob, t_left, t_right-0.01, 1e-03));
//            double spike = 1-exp(-prob(t_right, n));
//            p = p*no_spike*spike;
//            t_left = t_right+0.01;
//            out[yi+1] = p*exp(-DEIntegrator<double, SrmNeuron*>::Integrate(n, &prob, t_left, Tmax, 1e-03));
//        }
//        return;
//    }   



};

#endif
