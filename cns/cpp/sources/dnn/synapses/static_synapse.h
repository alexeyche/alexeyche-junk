#pragma once

#include "synapse.h"

namespace dnn {


struct StaticSynapseC {
    StaticSynapseC() : psp_decay(15.0), amp(1.0) {}

    double psp_decay;
    double amp;
};

struct StaticSynapseState {
    StaticSynapseState() : x(0.0) {}

    double x;
};

class StaticSynapse : public Synapse<StaticSynapseC, StaticSynapseState> {
public:
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
