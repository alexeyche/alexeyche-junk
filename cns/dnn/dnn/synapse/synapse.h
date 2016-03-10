#pragma once

namespace NDnn {

	struct TSynapseInnerState {
		TSynapseInnerState()
			: Fired(false) 
		{}

		size_t IdPre;
		double DendriteDelay;
		double Weight;
		double Potential;
		bool Fired;
		double Amplitude;
	};

	template <typename TConstants, typename TState>
	class TSynapse {
	public:
		TSynapse()
		{}

		bool& MutFired() {
			return InnerState.Fired;
		}

		double& MutAmplitude() {
			return InnerState.Amplitude;
		}
		const double& Amplitude() const {
			return InnerState.Amplitude;
		}


		double& MutPotential() {
			return InnerState.Potential;
		}
		const double& Potential() const {
			return InnerState.Potential;
		}
		
		double WeightedPotential() const {
			return InnerState.Weight * InnerState.Potential;
		}

	private:
		TSynapseInnerState InnerState;

	protected:
		
		TConstants c;
		TState s;
	};


} // namespace NDnn


