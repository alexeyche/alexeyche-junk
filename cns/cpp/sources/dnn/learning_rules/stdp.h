#pragma once


#include "learning_rule.h"
#include <dnn/protos/generated.pb.h>
#include <dnn/io/serialize.h>

namespace dnn {


/*@GENERATE_PROTO@*/
struct StdpC : public Serializable<Protos::StdpC> {
    StdpC() 
    : tau_plus(30.0)
    , tau_minus(50.0)
    , a_plus(1.0)
    , a_minus(1.5)
    , learning_rate(1.0)
    {}

    void serial_process() {
        begin() << "tau_plus: " << tau_plus << ", " 
                << "tau_minus: " << tau_minus << ", " 
                << "a_plus: " << a_plus << ", " 
                << "a_minus: " << a_minus << ", " 
                << "learning_rate: " << learning_rate << Self::end;
    }

    double tau_plus;
    double tau_minus;
    double a_plus;
    double a_minus;
    double learning_rate;
};


/*@GENERATE_PROTO@*/
struct StdpState : public Serializable<Protos::StdpState>  {
    StdpState() 
    : y(0.0)
    {}

    void serial_process() {
        begin() << "y: "        << y << ", " 
                << "x: "        << x << Self::end;
    }

    double y;
    vector<double> x;
};


class Stdp : public LearningRule<StdpC, StdpState> {
public:
    const string name() const {
        return "Stdp";
    }

    void reset() {
        s.y = 0;
        s.x.resize(n->getSynapses().size());
        for(auto &v: s.x) {
            v = 0;
        }
    }

    void propagateSynapseSpike(const SynSpike &sp) {
        s.x[sp.syn_id] += 1;
    }

    void calculateDynamics(const Time& t) {
        if(n->fired()) {
            s.y += 1;
        }
        vector<InterfacedPtr<SynapseBase>> &syns = n->getSynapses();
        for(size_t syn_id=0; syn_id < syns.size(); ++syn_id) {
            
            double dw = c.learning_rate * ( c.a_plus  * s.x[syn_id] * n->fired() -  \
                                            c.a_minus * s.y * syns[syn_id].ref().fired() );

            syns[syn_id].ref().weight += dw;
            s.x[syn_id] += - s.x[syn_id]/c.tau_plus;
        }
        s.y += - s.y/c.tau_minus;

        stat.add("y", s.y);

    }
    
};

}
