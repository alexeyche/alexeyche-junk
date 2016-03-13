#pragma once

#include <queue>
#include <atomic>

#include <dnn/util/act_vector.h>
#include <dnn/util/random.h>
#include <dnn/util/rand.h>
#include <dnn/util/serial/meta_proto_serial.h>
#include <dnn/protos/config.pb.h>



namespace NDnn {

	template <typename TNeuron, typename TConf>	
	class TSpikeNeuronImpl: public IMetaProtoSerial {
	public:
		TSpikeNeuronImpl()
			: InputSpikesLock(ATOMIC_FLAG_INIT)
		{}

		TSpikeNeuronImpl(const TSpikeNeuronImpl& other) {
			(*this) = other;
		}

		TSpikeNeuronImpl& operator = (const TSpikeNeuronImpl& other) {
			if (this != &other) {
				Neuron = other.Neuron;
				Activation = other.Activation;
				Synapses = other.Synapses;
			}
			return *this;
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
			if(Neuron.SpikeProbability() > Rand->GetUnif()) {
		        Neuron.MutFired() = true;
		        Neuron.PostSpikeDynamics(t);
		    }
		}

		TNeuron& GetNeuron() {
			return Neuron;
		}

		void SerialProcess(TMetaProtoSerial& serial) override final {
			serial(Neuron);
			serial(Activation);
			for (auto& synapse: Synapses) {
				serial(synapse);
			}
		}

		void SetRandEngine(TRandEngine& rand) {
			Rand.Set(rand);
		}

		void Prepare() {
			ENSURE(Rand, "Random engine is not set");

			Neuron.Reset();
		}

	private:
		TPtr<TRandEngine> Rand;

		TNeuron Neuron;

		typename TConf::TActivationFunction Activation;
		TActVector<typename TConf::TSynapse> Synapses;

		std::priority_queue<TSynSpike> InputSpikes;
		std::atomic_flag InputSpikesLock;	
	};

} // namespace NDnn