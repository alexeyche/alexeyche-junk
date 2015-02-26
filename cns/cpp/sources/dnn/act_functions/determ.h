#pragma once


#include "act_function.h"
#include <dnn/protos/generated.pb.h>

namespace dnn {

/*@GENERATE_PROTO@*/
struct DetermC : public Serializable<Protos::DetermC> {
    DetermC() : treshold(15.0) {}

    void processStream(Stream &str) {
        acquire(str) << "treshold: " << treshold << Self::End;
    }


    double treshold;
};


class Determ : public ActFunction<DetermC> {
public:
    double prob(const double &u) {
        if(u >= c.treshold) {
            return 1.0;
        }
        return 0.0;
    }
    double probDeriv(const double &u) {
        return 0.0;
    }

    void provideInterface(ActFunctionInterface &i) {
        i.prob = MakeDelegate(this, &Determ::prob);
        i.probDeriv = MakeDelegate(this, &Determ::probDeriv);
    }

};



}