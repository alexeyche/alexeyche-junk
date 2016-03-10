#pragma once

#include <dnn/util/serial.h>
#include <dnn/protos/spike_neuron.pb.h>


namespace NDnn {

	struct TSpikeNeuronInnerState: public IProtoSerial<NDnnProtos::TSpikeNeuronInnerState> {
		TSpikeNeuronInnerState()
			: Membrane(0.0)
			, SpikeProbability(0.0)
			, Fired(false)
		{}

		void SerialProcess(TProtoSerial& serial) override final {
            serial(Membrane);
            serial(SpikeProbability);
            serial(Fired);
        }

		double Membrane;
		double SpikeProbability;
		bool Fired;
	};

	template <typename TConstants, typename TState>
	class TSpikeNeuron: public ISerialStream {
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
		
		void SerialProcess(TSerialStream& serial) override final {
			serial(c); 
			serial(s);
			serial(InnerState);
		}

	private:
		TSpikeNeuronInnerState InnerState;
		
	protected:
		TState s;
		TConstants c;
	};
	
} // namespace NDnn