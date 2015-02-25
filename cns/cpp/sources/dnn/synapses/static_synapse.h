#pragma once


struct StaticSynapseC {
    double psp_decay;
    double amp;
};

struct StaticSynapseState {
    double x;
};

class StaticSynapse : public Synapse<StaticSynapseC, StaticSynapseState> {
public:
    void propagateSpike() {
        x += c.amp;
    }
    void calculateDynamics() {
        x -= x/c.psp_decay;
    }

    void provideInterface(SynapseInterface &i) {
        i.propagateSpike = MakeDelegate(this, &StaticSynapse::propagateSpike);
        i.calculateDynamics = MakeDelegate(this, &StaticSynapse::calculateDynamics);
    }
};
