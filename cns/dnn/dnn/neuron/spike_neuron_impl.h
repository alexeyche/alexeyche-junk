#pragma once

#include <queue>
#include <atomic>

#include <dnn/util/random.h>


namespace NDnn {

	template <typename TNeuron, typename TConf>	
	class TSpikeNeuronImpl {
	public:
		TSpikeNeuronImpl()
			: InputSpikesLock(ATOMIC_FLAG_INIT)
		{}

		void CalculateDynamicsInternal(const TTime& t) {
			ReadInputSpikes(t);
			
			// const double& Iinput = input.ifc().getValue(t);
			double Iinput = 0.0;

			double Isyn = 0.0;
		    for (const auto& synapse: Synapses) {
		    	double x = synapse.WeightedPotential();
		    	Isyn += x;
		    }
		    
		    Neuron.CalculateDynamics(t, Iinput, Isyn);

		    Neuron.MutSpikeProbability() = Activation.SpikeProbability(Neuron.Membrane());
			if(Neuron.SpikeProbability() > GetUnif()) {
		        Neuron.MutFired() = true;
		        Neuron.PostSpikeDynamics(t);
		    }
		}
		
		void ReadInputSpikes(const TTime &t) {
			while (InputSpikesLock.test_and_set(std::memory_order_acquire)) {}
		    while(!InputSpikes.empty()) {
		        const TSynSpike& sp = InputSpikes.top();
		        if(sp.T >= t.T) break;
		        auto& s = Synapses[sp.syn_id];
		        
		        s.MutFired() = true;
		    	s.PropagateSpike();

		        InputSpikes.pop();
		    }
		    InputSpikesLock.clear(std::memory_order_release);
		}
		
		TNeuron& GetNeuron() {
			return Neuron;
		}

	private:
		TNeuron Neuron;

		typename TConf::TActivationFunction Activation;
		std::vector<typename TConf::TSynapse> Synapses;

		std::priority_queue<TSynSpike> InputSpikes;
		std::atomic_flag InputSpikesLock;	
	};

} // namespace NDnn