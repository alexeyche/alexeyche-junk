#include "random.h"

#include <cstdlib>
#include <cmath>

namespace NDnn {

    double GetUnif() {
        return (double)rand()/(double)RAND_MAX;
    }


    double GetExp(double rate) {
        double u = GetUnif();
        return -log(u)/rate;
    }

    double normal_distr_var = -1;

    double GetNorm() {
        if(normal_distr_var<0) {
            double U = GetUnif();
            double V = GetUnif();
            normal_distr_var = sqrt(-2*log(U)) * cos(2*PI*V);
            return(sqrt(-2*log(U)) * sin(2*PI*V));
        } else {
            double ret = normal_distr_var;
            normal_distr_var=-1;
            return(ret);
        }
    }

    double GetLogNorm(double logmean, double logsd) {
        return( exp(logmean + logsd*GetNorm()) );
    }

} // namespace NDnn

