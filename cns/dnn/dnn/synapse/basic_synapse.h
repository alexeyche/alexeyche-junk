#pragma once

#include "synapse.h"

namespace NDnn {


struct TBasicSynapseC {
    TBasicSynapseC() : PspDecay(15.0), Amp(1.0) {}

    double PspDecay;
    double Amp;
};

struct TBasicSynapseState {};

class TBasicSynapse : public TSynapse<TBasicSynapseC, TBasicSynapseState> {
public:

    void Reset() {
        MutAmplitude() = c.Amp;
        MutPotential() = 0;
    }

    void CalculateDynamics(const TTime &t) {
        MutPotential() += - t.Dt * Potential()/c.PspDecay;
    }

	void PropagateSpike() {
	    MutPotential() += Amplitude();
	}

};




}
