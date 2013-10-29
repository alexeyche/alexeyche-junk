

#include <sim/core.h>
#include "srm.h"

namespace srm {
    const double ts = 3; // ms
    const double tm = 10; // ms
    
    double epsp(double t, double fj, double fi) {
        if( ((t-fj)<0) ||  (fj<0) ) return 0.0;
        return (exp( -std::max(fi-fj, 0.0)/ts )/(1-ts/tm)) * (exp(-std::min(t-fi,t-fj)/tm) - exp(-std::min(t-fi,t-fj)/ts));
    }



    const double u_abs = -100; // mV
    const double u_r = -50; // mV
    const double trf = 0.25; // ms
    const double trs = 3; // ms
    const double dr = 1; // ms

    double nu(double t, double fi) {
        if((t-fi)<0) return 0;
        if((t-fi)<dr) return u_abs;
        return u_abs*exp(-(t-fi+dr)/trf)+u_r*exp(-(t-fi)/trs);
    }
    

};
