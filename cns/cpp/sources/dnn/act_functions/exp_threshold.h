#pragma once


#include "act_function.h"

#include <dnn/protos/generated.pb.h>
#include <dnn/util/fastapprox/fastexp.h>
#include <dnn/util/fastapprox/fastlog.h>

namespace dnn {

/*@GENERATE_PROTO@*/
struct ExpThresholdC : public Serializable<Protos::ExpThresholdC> {
    ExpThresholdC() 
    : threshold(15.0)
    , p_rest(0.1)
    , beta(2.0)
    , amp(10.0) 
    {        
    }

    void serial_process() {
        begin() << "beta: " << beta << ", " 
                << "p_rest: " << p_rest << ", "
                << "amp: " << amp << ", "
                << "threshold: " << threshold << Self::end;
    }

    double amp;
    double beta;
    double threshold;
    double p_rest;
};


class ExpThreshold : public ActFunction<ExpThresholdC> {
public:
    const string name() const {
        return "ExpThreshold";
    }
    double prob(const double &u) {        
        double p = (c.p_rest + c.amp * fastlog(1.0 + fastexp( c.beta*(u - c.threshold) )))/1000.0;
        if(p>1.0) return 1.0;
        return p;
    }
    double probDeriv(const double &u) {
        double exp_part = c.beta*(u - c.threshold);
        double part;
        if(exp_part > 10.0) {
            part = 22026.47;
        } else {
            part = fastexp(exp_part);
        }
        return ( (part*c.amp*c.beta) / ( 1.0+part))/1000.0;
    }
};



}
