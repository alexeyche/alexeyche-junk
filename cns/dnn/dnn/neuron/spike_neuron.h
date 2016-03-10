#pragma once

namespace NDnn {

	struct TSpikeNeuronInnerState {
		TSpikeNeuronInnerState()
			: Membrane(0.0)
			, SpikeProbability(0.0)
			, Fired(false)
		{}

		double Membrane;
		double SpikeProbability;
		bool Fired;
	};

	template <typename TConstants, typename TState>
	class TSpikeNeuron {
	public:
		TSpikeNeuron()
			
		{}

		double& MutMembrane() {
			return InnerState.Membrane;
		}
		
		const double& Membrane() const {
			return InnerState.Membrane;
		}

		bool& MutFired() {
			return InnerState.Fired;
		}
		
		const bool& Fired() const {
			return InnerState.Fired;
		}

		double& MutSpikeProbability() {
			return InnerState.SpikeProbability;
		}
		
		const double& SpikeProbability() const {
			return InnerState.SpikeProbability;
		}

	private:
		TSpikeNeuronInnerState InnerState;
		
	protected:
		TState s;
		TConstants c;
	};
	
} // namespace NDnn