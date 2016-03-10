#pragma once

#include "spike_neuron.h"

#include <dnn/util/serial.h>
#include <dnn/protos/integrate_and_fire.pb.h>


namespace NDnn {

	struct TIntegrateAndFireConst: public IProtoSerial<NDnnProtos::TIntegrateAndFireConst> {
	    TIntegrateAndFireConst()
	    	: TauMem(5.0)
		    , RestPotential(0.0)
		    , TauRef(2.0)
		    , NoiseAmp(0.0)
	    {}

        void SerialProcess(TProtoSerial& serial) override final {
            serial(TauMem);
            serial(RestPotential);
            serial(TauRef);
            serial(NoiseAmp);
        }


	    double TauMem;
	    double RestPotential;
	    double TauRef;
	    double NoiseAmp;
	};


	struct TIntegrateAndFireState: public IProtoSerial<NDnnProtos::TIntegrateAndFireState> {
	    TIntegrateAndFireState()
	        : RefTime(0.0)
	    {}

	    void SerialProcess(TProtoSerial& serial) override final {
            serial(RefTime);
        }

	    double RefTime;
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

	    void CalculateDynamics(const TTime& t, const double &Iinput, const double &Isyn) {
	        if(s.RefTime < 0.001) {
	            MutMembrane() += t.Dt * ( - Membrane()  + c.NoiseAmp*0.0 + Iinput + Isyn) / c.TauMem;
	        } else {
	            s.RefTime -= t.Dt;
	        }
	    }
	};

} // namespace NDnn

