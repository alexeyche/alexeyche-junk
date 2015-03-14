#pragma once

#include "synapse.h"
#include <dnn/protos/generated.pb.h>
#include <dnn/io/serialize.h>

namespace dnn {


/*@GENERATE_PROTO@*/
struct StaticSynapseC : public Serializable<Protos::StaticSynapseC>  {
    StaticSynapseC() : psp_decay(15.0), amp(1.0) {}
    
    void serial_process() {
        begin() << "psp_decay: " << psp_decay << ", " \
                << "amp: "       << amp       << Self::end;
    }
    
    double psp_decay;
    double amp;
};

/*@GENERATE_PROTO@*/
struct StaticSynapseState : public Serializable<Protos::StaticSynapseState>  {
    StaticSynapseState() : x(0.0) {}

    void serial_process() {
        begin() << "x: " << x << Self::end;
    }

    double x;
};

class StaticSynapse : public Synapse<StaticSynapseC, StaticSynapseState> {
public:
    const string name() const {
        return "StaticSynapse";
    }
    void propagateSpike() {
        s.x += c.amp;
    }
    void calculateDynamics(const Time &t) {
        s.x -= t.dt * s.x/c.psp_decay;
    }

    void provideInterface(SynapseInterface &i) {
        i.propagateSpike = MakeDelegate(this, &StaticSynapse::propagateSpike);
        i.calculateDynamics = MakeDelegate(this, &StaticSynapse::calculateDynamics);
    }
};




}
