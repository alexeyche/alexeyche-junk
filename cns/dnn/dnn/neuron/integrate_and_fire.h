#pragma once

#include "spike_neuron.h"

#include <dnn/util/serial/proto_serial.h>
#include <dnn/protos/integrate_and_fire.pb.h>
#include <dnn/protos/config.pb.h>

namespace NDnn {

	struct TIntegrateAndFireConst: public IProtoSerial<NDnnProto::TIntegrateAndFireConst> {
		static const auto ProtoFieldNumber = NDnnProto::TLayer::kIntegrateAndFireConstFieldNumber;

        void SerialProcess(TProtoSerial& serial) override final {
            serial(TauMem);
            serial(RestPotential);
            serial(TauRef);
            serial(NoiseAmp);
        }


	    double TauMem = 5.0;
	    double RestPotential = 0.0;
	    double TauRef = 2.0;
	    double NoiseAmp = 0.0;
	};


	struct TIntegrateAndFireState: public IProtoSerial<NDnnProto::TIntegrateAndFireState> {
		static const auto ProtoFieldNumber = NDnnProto::TLayer::kIntegrateAndFireStateFieldNumber;

	    void SerialProcess(TProtoSerial& serial) override final {
            serial(RefTime);
        }

	    double RefTime = 0.0;
	};

	class TIntegrateAndFire : public TSpikeNeuron<TIntegrateAndFireConst, TIntegrateAndFireState> {
	public:
		void Reset() {
	        MutMembrane() = c.RestPotential;
	        MutSpikeProbability() = 0.0;
	        s.RefTime = 0.0;
	    }

	    void PostSpikeDynamics(const TTime& t) {
	        MutMembrane() = c.RestPotential;
	        s.RefTime = c.TauRef;
	    }

	    void CalculateDynamics(const TTime& t, double Iinput, double Isyn) {
	        if(s.RefTime < 0.001) {
	            MutMembrane() += t.Dt * ( - Membrane()  + c.NoiseAmp * Rand->GetNorm() + Iinput + Isyn) / c.TauMem;
	        } else {
	            s.RefTime -= t.Dt;
	        }
	    }
	};

} // namespace NDnn
