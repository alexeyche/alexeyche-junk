#pragma once

#include "synapse.h"

namespace NDnn {


struct TBasicSynapseConst: public IProtoSerial<NDnnProto::TBasicSynapseConst> {
    static const auto ProtoFieldNumber = NDnnProto::TLayer::kBasicSynapseConstFieldNumber;

    void SerialProcess(TProtoSerial& serial) override {
        serial(PspDecay);
        serial(Amp);
    }

    double PspDecay = 15.0;
    double Amp = 1.0;
};

struct TBasicSynapseState: public IProtoSerial<NDnnProto::TBasicSynapseState> {
    static const auto ProtoFieldNumber = NDnnProto::TLayer::kBasicSynapseStateFieldNumber;

    void SerialProcess(TProtoSerial& serial) override {}
};

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
