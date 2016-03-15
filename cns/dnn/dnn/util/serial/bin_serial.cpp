#include "bin_serial.h"

#include <dnn/util/ts/time_series.h>
#include <dnn/util/ts/spikes_list.h>

namespace NDnn {

    TBinSerial::TBinSerial(std::ostream& ostr) 
        : TSerialBase(ESerialMode::OUT) 
        , OStr(&ostr)
    {
    	ENSURE(OStr->good(), "Output stream for TBinSerial is not good");

    	ZeroOut = new NPbIO::OstreamOutputStream(OStr);
        CodedOut = new NPbIO::CodedOutputStream(ZeroOut);
    }

    TBinSerial::TBinSerial(std::istream& istr)
        : TSerialBase(ESerialMode::IN) 
        , IStr(&istr)
    {
    	ENSURE(IStr->good(), "Input stream for TBinSerial is not good");

        ZeroIn = new NPbIO::IstreamInputStream(IStr);
        CodedIn = new NPbIO::CodedInputStream(ZeroIn);
        CodedIn->SetTotalBytesLimit(300.0 * 1024 * 1024, 300.0 * 1024 * 1024);
    } 

    TBinSerial::~TBinSerial() {
    	if (Mode == ESerialMode::IN) {
    		delete CodedIn;
            delete ZeroIn;
    	} else 
    	if (Mode == ESerialMode::OUT) {
    		delete CodedOut;
            delete ZeroOut;
    	}
    }

    EProto TBinSerial::ReadProtobufType() {
        ENSURE(Mode == ESerialMode::IN, "Can't get protobuf type from non input stream");

        NPb::uint32 type;
        ENSURE(CodedIn->ReadVarint32(&type), "Failed to read protobuf type from stream");
        return static_cast<EProto>(type); 
    }

    void TBinSerial::ReadProtobufMessage(NPb::Message& message) {
        NPb::uint32 size;
        ENSURE(CodedIn->ReadVarint32(&size), "Stream prematurely ended while reading size of message");

        NPbIO::CodedInputStream::Limit limit = CodedIn->PushLimit(size);
        ENSURE(message.ParseFromCodedStream(CodedIn), "Can't parse message with size " << size);
        CodedIn->PopLimit(limit);
    }

	bool TBinSerial::operator ()(NPb::Message& message, EProto protoType) {
		switch (Mode) {
            case ESerialMode::IN:
            {
                NPb::uint32 type;
                if (!CodedIn->ReadVarint32(&type)) {
                    return false;
                }
                EProto readType = static_cast<EProto>(type);
		        ENSURE(readType == protoType,
                    "Got different type of message from input: " << static_cast<ui32>(readType) << ", expected " << static_cast<ui32>(protoType));
                ReadProtobufMessage(message);
            }
            break;
            case ESerialMode::OUT:
            {
            	CodedOut->WriteVarint32(static_cast<ui32>(protoType));
            	CodedOut->WriteVarint32(message.ByteSize());
            	message.SerializeToCodedStream(CodedOut);
            }
            break;
        }
        return true;
	}

    template <>
    EProto TBinSerial::DeduceType<TTimeSeries>() {
        return EProto::TIME_SERIES;
    }

    template <>
    EProto TBinSerial::DeduceType<TSpikesList>() {
        return EProto::SPIKES_LIST;
    }

} // namespace NDnn
