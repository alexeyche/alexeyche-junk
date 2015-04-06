#pragma once


#include "act_function.h"
#include <dnn/protos/generated.pb.h>

namespace dnn {

/*@GENERATE_PROTO@*/
struct DetermC : public Serializable<Protos::DetermC> {
    DetermC() : treshold(15.0) {}

    void serial_process() {
        begin() << "treshold: " << treshold << Self::end;
    }


    double treshold;
};


class Determ : public ActFunction<DetermC> {
public:
    const string name() const {
        return "Determ";
    }
    double prob(const double &u) {        
        if(u >= c.treshold) {
            return 1.0;
        }
        return 0.0;
    }
    double probDeriv(const double &u) {
        return 0.0;
    }
};



}
