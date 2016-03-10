#pragma once

#include "synapse.h"

namespace NDnn {


struct TBasicSynapseConst {
    TBasicSynapseConst() : PspDecay(15.0), Amp(1.0) {}

    double PspDecay;
    double Amp;
};

struct TBasicSynapseState {};

class TBasicSynapse : public TSynapse<TBasicSynapseConst, TBasicSynapseState> {
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
