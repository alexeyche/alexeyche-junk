#pragma once

namespace NDnn {

	struct TSynapseInnerState: public IProtoSerial<NDnnProto::TSynapseInnerState> {
		
		void SerialProcess(TProtoSerial& serial) override {
			serial(IdPre);
			serial(DendriteDelay);
			serial(Weight);
			serial(Potential);
			serial(Fired);
			serial(Amplitude);
		}

		size_t IdPre = 0;
		double DendriteDelay = 0.0;
		double Weight = 1.0;
		double Potential = 0.0;
		bool Fired = false;
		double Amplitude = 1.0;
	};

	template <typename TConstants, typename TState>
	class TSynapse: public IProtoSerial<NDnnProto::TLayer> {
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

		void SerialProcess(TProtoSerial& serial) override final {
			serial(c, TConstants::ProtoFieldNumber); 
			serial(s, TState::ProtoFieldNumber);
			serial(InnerState, NDnnProto::TLayer::kSynapseInnerStateFieldNumber);
		}

	private:
		TSynapseInnerState InnerState;

	protected:
		
		TConstants c;
		TState s;
	};


} // namespace NDnn


