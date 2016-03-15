#pragma once

#include "serial_base.h"
#include "proto_serial.h"

#include <google/protobuf/message.h>
#include <google/protobuf/io/zero_copy_stream_impl.h>
#include <google/protobuf/io/coded_stream.h>


namespace NPb = google::protobuf;
namespace NPbIO = google::protobuf::io;


namespace NDnn {

	enum class EProto {
		CONFIG = 0,
		TIME_SERIES = 1,
		SPIKES_LIST = 2
	};

	class TBinSerial: public TSerialBase {
	public:
        TBinSerial(std::ostream& ostr);
        TBinSerial(std::istream& istr);
 		~TBinSerial();

		EProto ReadProtobufType();
		
		template <typename T>
		T ReadObject() {
			typename T::TProto pb;
	        ReadProtobufMessage(pb);
	        T obj;
	        obj.Deserialize(pb);
	        return obj;
		}
		
		void ReadProtobufMessage(NPb::Message& message);

		template <typename T>
		bool WriteObject(T&& obj) {
			typename T::TProto proto = obj.Serialize();
			ENSURE((*this)(proto, DeduceType<T>()), "Failed to write object into stream");
			return true;
		}

		bool operator ()(NPb::Message& message, EProto protoType);

		template <typename T>
		EProto DeduceType();

	private:
        std::ostream* OStr;
        std::istream* IStr;

		NPbIO::OstreamOutputStream* ZeroOut;
	    NPbIO::CodedOutputStream* CodedOut;

	    NPbIO::IstreamInputStream* ZeroIn;
	    NPbIO::CodedInputStream* CodedIn;
	};

} // namespace NDnn