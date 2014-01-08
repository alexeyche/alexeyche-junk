
#include "neurons.h"

namespace srm {
    
// SrmNeuron functions:    
    
    double SrmNeuron::epsp(double t, double fj, double fi) {
        if( ((t-fj)<0) ||  (fj<0) ) return 0.0;
        if(fi>t) fi = -datum::inf;
        return (exp( -std::max(fi-fj, 0.0)/ts )/(1-ts/tm)) * (exp(-std::min(t-fi,t-fj)/tm) - exp(-std::min(t-fi,t-fj)/ts));
    }
    
    double SrmNeuron::nu(double t, double fi) {
        if((t-fi)<0) return 0;
        if((t-fi)<dr) return u_abs;
        return u_abs*exp(-(t-fi+dr)/trf)+u_r*exp(-(t-fi)/trs);
    }
   
    #define APLUS 1.5
    #define AMINUS 1.5
    #define TPLUS 10
    #define TMINUS 10
    #define STDP_RATE 0.001
    #define MU 0.7
    double SrmNeuron::u(const double &t, TTime &y_given) {
        double epsp_pot = 0;
                    
        double &y_last = y_given.last(t-0.001);
//            printf("============================================\n");  
//            printf("y_last: %f\n", y_last); 
//            Log::Info << "neuron " << id() << " in.size() == " << in.size() << "\n";
        for(size_t i=0; i<in.size(); i++) {
            for(int j=(in[i]->y.n_elem(t)-1); j>=0; j--) {
//                printf("epsp_pot: %e\n", epsp_pot);
//                printf(" w: %e t: %f in.y(j): %f y: %f\n", w[i], t, in[i]->y(j), y_last);
                if( (t - in[i]->y(j)) > EPSP_WORK_WINDOW) {
                    //printf("epsp ignoring: %e\n", epsp(t, in[i]->y(j), y_last));
                    continue;
                }                        
                epsp_pot += w[i]*epsp(t, in[i]->y(j), y_last);
            }
            if(stdp_learning) {
                double dt = in[i]->y.last(t) - y_last;
                if(abs(dt) < 20) { 
                   double dw=0;
                   if(dt<=0) {
                       if(w[i]<4)
                       dw = pow(4-w[i], MU)*APLUS*exp(dt/TPLUS);
                   } else {
                       if(w[i]>-2)
                       dw = -pow(2+w[i], MU)*AMINUS*exp(-dt/TMINUS);
                   }
                   //Log::Info << "syn " << i << " dt " << dt << "(" << in[i]->y.last(t) << ")" << " dw = " << dw << " w[i] " << w[i] << " \n"; 
                   double mod_w = STDP_RATE*dw;
                   //Log::Info << "syn " << i << " dt " << dt << "(" << in[i]->y.last(t) << ")" << " dw = " << dw << " \n"; 
                   w[i] += mod_w;
                }
            }
        }
        double nu_pot = 0;
        for(int i = (y_given.n_elem(t-0.001)-1); i>=0; i--) {
            if( (t-y_given(i)) > NU_WORK_WINDOW ) {
                //printf("nu ignoring(%f): %e\n", t, nu(t, y_given(i)));
                break;
            }                    
            nu_pot += nu(t, y_given(i));
        }
        return u_rest + epsp_pot + nu_pot;           
    }

    double SrmNeuron::u(const double &t) {
        return u(t, y); 
    }

    double SrmNeuron::p(const double &t, TTime &y_given) {
        double uc = u(t, y_given);
//        Log::Info << "//===============================\n";
//        Log::Info << "uc: " << uc << "\n";
        double p = (beta/alpha)*(log(1+exp(alpha*(tresh-uc))) - alpha*(tresh-uc));           
//        Log::Info << "p: " << p << "\n";
        return p;
    }
    double SrmNeuron::p(const double &t) {
        return p(t, y);           
    }
  


};    
