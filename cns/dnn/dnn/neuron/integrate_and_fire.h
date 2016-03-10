#pragma once

#include "spike_neuron.h"

namespace NDnn {

	struct TIntegrateAndFireC {//: public ISerial<Protos::IntegrateAndFireC> {
	    TIntegrateAndFireC()
	    	: TauMem(5.0)
		    , RestPotential(0.0)
		    , TauRef(2.0)
		    , NoiseAmp(0.0)
	    {}


	    double TauMem;
	    double RestPotential;
	    double TauRef;
	    double NoiseAmp;
	};


	struct TIntegrateAndFireState {//: public Serializable<Protos::IntegrateAndFireState>  {
	    TIntegrateAndFireState()
	        : RefTime(0.0)
	    {}

	    double RefTime;
	};

	class TIntegrateAndFire : public TSpikeNeuron<TIntegrateAndFireC, TIntegrateAndFireState> {
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

