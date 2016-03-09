#pragma once


namespace NDnn {

	struct TInnerState {
		TInnerState()
			: Membrane(0.0)
			, SpikeProbability(0.0)
		{}

		double Membrane;
		double SpikeProbability;
	};


	template <typename TConstants, typename TState>
	class TSpikeNeuron {
	public:


		double& MutMembrane() {
			return InnerState.Membrane;
		}
		
		const double& Membrane() const {
			return InnerState.Membrane;
		}

		double& MutSpikeProbability() {
			return InnerState.SpikeProbability;
		}
		
		const double& SpikeProbability() const {
			return InnerState.SpikeProbability;
		}
	private:
		TInnerState InnerState;
	
	protected:
		TState s;

		TConstants c;
	};

}