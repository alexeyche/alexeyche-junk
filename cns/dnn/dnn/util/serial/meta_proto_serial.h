#pragma once

#include "serial_base.h"

#include <google/protobuf/message.h>

namespace NPb = google::protobuf;;


namespace NDnn {

	class IMetaProtoSerial;

    template <typename T>
    class IProtoSerial;

    class TMetaProtoSerial: public TSerialBase {
    public:
        TMetaProtoSerial(NPb::Message& message, int dstFieldNumber, ESerialMode mode);

        template <typename T>
        void operator() (IProtoSerial<T>& v);

        void operator() (IMetaProtoSerial& v);
        
        void operator() (NPb::Message& m, int protoField);
        
        const NPb::FieldDescriptor* GetFieldDescr(int protoField);
        
        void DuplicateSingleRepeated(ui32 duplicateFactor);

    private:
        const NPb::Reflection* Refl;
        const NPb::Descriptor* Descr;

        NPb::Message& Message;
        int DstFieldNumber;
        ui32 DuplicateFactor;
    };

	
	class IMetaProtoSerial {
    public:
        virtual void SerialProcess(TMetaProtoSerial& serial) = 0;
    };


} // namespace NDnn

#include "meta_proto_serial-inl.h"