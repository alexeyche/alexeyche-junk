#ifndef RESEARCH_H
#define RESEARCH_H

#include <sim/int/DEIntegrator.h>


#include "neurons.h"

namespace srm {
    class TEntropyGrad {
    public:
        TEntropyGrad(SrmNeuron *n_v) : n(n_v) {} 
        double p_stroke(double &t) {
            return SrmNeuron::beta/( 1 + exp(SrmNeuron::alpha*(SrmNeuron::tresh - n->u(t))) );
        }
//        static vec grab_epsp(const double &t, const int &index) {
//            double &y_last = n->y.last(t-0.001);
//            double epsp_pot = 0;
//            for(int j=(n->in[index]->y.n_elem(t)-1); j>=0; j--) {
//                if( (t - n->in[index]->y(j)) > EPSP_WORK_WINDOW) { continue; }
//                epsp_pot += SrmNeuron::epsp(t, n->in[index]->y(j), y_last);
//            }    
//            return epsp_out;
//        }
//        double gradNoSpike() {
//            vec grad_w(n->w.size());
//            vec ep = grab_epsp(10, n); 
//            double ep_int = DEIntegrator<double, const int&>::Integrate(0, &grab_epsp, 0, 20, 1e-03);
//            Log::Info << ep_int << "\n";
//        }
        SrmNeuron *n;
    };

    double prob(const double &t, SrmNeuron *n) {
       return n->p(t);
    }
    
    double survFunction(SrmNeuron *n, double T0, double Tmax) {
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
