#pragma once

#include "synapse.h"
#include <dnn/protos/generated.pb.h>
#include <dnn/io/serialize.h>

namespace dnn {


/*@GENERATE_PROTO@*/
struct STDSynapseC : public Serializable<Protos::STDSynapseC>  {
    STDSynapseC() : psp_decay(2.0), amp(1.0), gamma(0.65), tau_d(400.0) {}
    
    void serial_process() {
        begin() << "psp_decay: " << psp_decay << ", "
                << "amp: "       << amp     << ", " 
                << "gamma: "     << gamma   << ", " 
                << "tau_d: "     << tau_d   << Self::end;
    }
    
    double psp_decay;
    double amp;
    double gamma;
    double tau_d;
};

/*@GENERATE_PROTO@*/
struct STDSynapseState : public Serializable<Protos::STDSynapseState>  {
    STDSynapseState() : x(0.0), res(1.0) {}

    void serial_process() {
        begin() << "x: "    << x   << ", "
                << "res: "  << res << Self::end;
    }

    double x;
    double res;
};

class STDSynapse : public Synapse<STDSynapseC, STDSynapseState> {
public:
    const string name() const {
        return "STDSynapse";
    }
    
    void reset() {
        s.x = 0.0;
        s.res = 1.0;
    }

    void propagateSpike() {
        s.x += c.amp * s.res;
        s.res -= (1 - c.gamma) * s.res;
    }
    void calculateDynamics(const Time &t) {
        stat.add("x", s.x);
        stat.add("res", s.res);
        
        s.x += - t.dt * s.x/c.psp_decay;   
        s.res += (1 - s.res)/c.tau_d;
    }

    double getMembranePotential() {
        return weight() * s.x;
    }
};




}
