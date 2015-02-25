#pragma once

#include "spike_neuron.h"


/*@GENERATE_PROTO@*/
struct LeakyIntegrateAndFireC : public Constants {
	double R;
	double C;
};


/*@GENERATE_PROTO@*/
struct LeakyIntegrateAndFireState  {
	double p;
	double u;
};


class LeakyIntegrateAndFire : public SpikeNeuron<LeakyIntegrateAndFireC, LeakyIntegrateAndFireState> {
public:
    void reset() {
        s.p = 0.0;
        s.u = 0.0;
    }

    void propagateSynapseSpike(const SynSpike &sp) {
        syns[ sp.syn_id ]->propagateSpike();
    }
};
