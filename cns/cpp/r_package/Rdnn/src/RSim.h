#ifndef RSIM_H
#define RSIM_H


#include <dnn/sim/sim.h>

#undef PI
#define STRICT_R_HEADERS
#include <Rcpp.h>

#include "RConstants.h"
#include "RProto.h"

class RSim : public dnn::Sim {
public:
    RSim(RConstants *rc) : Sim(*rc)  {
//        global_neuron_index = 0;
    }
    ~RSim() { }
    void print() {
        cout << *this;
    }
    void run(size_t jobs=1) {
        Sim::run(jobs);
    }
    Rcpp::List getStat() {
        Rcpp::List out;
        for(auto &n: neurons) {
            if(n.ref().getStat().on()) {
                stringstream ss;
                ss << n.ref().name() << "_" << n.ref().id();
                out[ss.str()] = RProto::convertToList(&n.ref().getStat());
            }
        }
        return out;
    }
};



#endif
