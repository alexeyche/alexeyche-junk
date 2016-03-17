#pragma once

#include <dnn/util/serial/proto_serial.h>
#include <dnn/protos/spike_neuron.pb.h>
#include <dnn/protos/config.pb.h>

namespace NDnn {

	struct TSpikeNeuronInnerState: public IProtoSerial<NDnnProto::TSpikeNeuronInnerState> {
		void SerialProcess(TProtoSerial& serial) override final {
            serial(Membrane);
            serial(SpikeProbability);
            serial(Fired);
        }

		double Membrane = 0.0;
		double SpikeProbability = 0.0;
		bool Fired = false;
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

		void SetSpikeSequence(const TVector<double>&) {
			throw TDnnException() << "Trying to set spike sequence on neuron without appropriate method";
		}
		const TState& State() const {
			return s;
		}

		void SetRandEngine(TRandEngine& rand) {
			Rand.Set(rand);
		}
	private:
		TSpikeNeuronInnerState InnerState;

	protected:
		TPtr<TRandEngine> Rand;

		TState s;
		TConstants c;
	};

} // namespace NDnn