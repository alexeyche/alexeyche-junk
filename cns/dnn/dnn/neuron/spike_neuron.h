#pragma once

#include <dnn/util/serial/proto_serial.h>
#include <dnn/protos/spike_neuron.pb.h>
#include <dnn/protos/config.pb.h>

namespace NDnn {

	struct TSpikeNeuronInnerState: public IProtoSerial<NDnnProto::TSpikeNeuronInnerState> {
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
	class TSpikeNeuron: public IProtoSerial<NDnnProto::TLayer> {
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
		
		void SerialProcess(TProtoSerial& serial) override final {
			serial(c, TConstants::ProtoFieldNumber); 
			serial(s, TState::ProtoFieldNumber);
			serial(InnerState, NDnnProto::TLayer::kSpikeNeuronInnerStateFieldNumber);
		}

	private:
		TSpikeNeuronInnerState InnerState;
		
	protected:
		TState s;
		TConstants c;
	};
	
} // namespace NDnn