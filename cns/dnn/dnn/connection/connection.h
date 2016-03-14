#pragma once

#include <dnn/util/serial/proto_serial.h>

#include <dnn/protos/config.pb.h>

#include <dnn/util/rand.h>

namespace NDnn {

	struct TConnectionRecipe {
		bool Inhibitory = false;
		double Amplitude = 1.0;
		bool Exists = false;
	};

	struct TNeuronSpaceInfo;

	class IConnection: public IProtoSerial<NDnnProto::TConnection> {
	public:
		virtual ~IConnection() {}

		virtual TConnectionRecipe GetConnectionRecipe(const TNeuronSpaceInfo& left, const TNeuronSpaceInfo& right) = 0;

		void SetRandEngine(TRandEngine& rand) {
			Rand.Set(rand);
		}

	protected:
		TPtr<TRandEngine> Rand;
	};

	template <typename TConstants>
	class TConnection: public IConnection {
	public:
		TConnection() {}
		
		void SerialProcess(TProtoSerial& serial) override final {
			serial(c, TConstants::ProtoFieldNumber); 
		}

	
	protected:
		TConstants c;
	};

	using TConnectionPtr = SPtr<IConnection>;

} // namespace NDnn


