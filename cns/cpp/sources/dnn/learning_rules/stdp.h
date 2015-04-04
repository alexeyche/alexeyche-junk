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
        for(auto &v: s.x) {
            v = 0;
        }
    }

    void propagateSynapseSpike(const SynSpike &sp) {
    }

    void calculateDynamics(const Time& t) {
        
        stat.add("y", s.y);
    }

    void provideInterface(LearningRuleInterface &i) {
        i.calculateDynamics = MakeDelegate(this, &Stdp::calculateDynamics);
        i.propagateSynapseSpike = MakeDelegate(this, &Stdp::propagateSynapseSpike);
    }
};

}
