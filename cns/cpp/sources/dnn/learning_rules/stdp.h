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
        auto &syns = n->getSynapses();
        for(auto syn_id_it = syns.ibegin(); syn_id_it != syns.iend(); ++syn_id_it) {
            auto &syn = syns[syn_id_it].ref();
            double dw = c.learning_rate * ( c.a_plus  * s.x[*syn_id_it] * n->fired() -  \
                                            c.a_minus * s.y * syn.fired() );

            syn.getMutWeight() += dw;
            s.x[*syn_id_it] += - s.x[*syn_id_it]/c.tau_plus;
        }
        s.y += - s.y/c.tau_minus;
        
        if(stat.on()) {
            size_t i=0; 
            for(auto &syn: syns) {
                stat.add("x", i, s.x[i]);
                stat.add("w", i, syn.ref().getWeight());
                ++i;
            }
            stat.add("y", s.y);
        }
    }
    
};

}
