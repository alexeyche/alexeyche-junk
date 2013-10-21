

namespace srm {
    const double ts = 0.7; // ms
    const double tm = 10; // ms
    
    double epsp(double t, double fj, double fi) {
        if( ((t-fj)<0) || ((t-fi)<0) || (fj<0) ) return 0.0;
        return (exp( -std::max(fi-fj, 0.0)/ts )/(1-ts/tm)) * (exp(-std::min(t-fi,t-fj)/tm) - exp(-std::min(t-fi,t-fj)/ts));
    }
};



