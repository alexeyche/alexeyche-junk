
#include "neurons_new.h"

namespace srm {

    double SrmNeuron2::epsp(double t, double fj) {
        double s = t - fj;
        if(s<0) return 0;
        return exp(-s/tm)-exp(-s/ts);
    }


    double SrmNeuron2::nu(double t, double fi) {
        double s = t-fi;
        if(s<0) return 0;
        if(s<dr) return u_abs;
        return u_abs*exp(-(s+dr)/trf)+u_r*exp(-s/trs);
    }

    
    double SrmNeuron2::u(const double &t, TTime &y_given) {
//        __m128 mmSum = _mm_setzero_ps();
    }
    
    double SrmNeuron2::u(const double &t) {
        return u(t, y);
    }

    double SrmNeuron2::p(const double &t, TTime &y_given) {
        double uc = u(t, y_given);
        return (beta/alpha)*(log(1+exp(alpha*(tresh-uc))) - alpha*(tresh-uc));           
    }
    double SrmNeuron2::p(const double &t) {
        return p(t, y);           
    }

};
