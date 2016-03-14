#pragma once

#include "serial_base.h"

#include <dnn/util/log/log.h>

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
        bool operator() (IProtoSerial<T>& v);

        bool operator() (IMetaProtoSerial& v);
        
        bool operator() (NPb::Message& m, int protoField);
        
        const NPb::FieldDescriptor* GetFieldDescr(int protoField);
        
        void DuplicateSingleRepeated(ui32 duplicateFactor);

        bool HasField(int protoField);

        template <typename T>
        const T& GetMessage() const {
            try {
                return dynamic_cast<T&>(Message);
            } catch (const std::bad_cast& error) {
                T m;
                L_ERROR << "Failed to cast " << Message.GetTypeName() << " into " << m.GetTypeName();
                throw; 
            }
        }

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