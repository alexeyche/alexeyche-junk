#pragma once

#include <dnn/util/serial/proto_serial.h>

#include <dnn/protos/config.pb.h>

namespace NDnn {


	template <typename TConstants>
	class TActivation: public IProtoSerial<NDnnProto::TLayer> {
	protected:
		TActivation() {}
		
		void SerialProcess(TProtoSerial& serial) override final {
			serial(c, TConstants::ProtoFieldNumber); 
		}

		TConstants c;
	};


} // namespace NDnn


