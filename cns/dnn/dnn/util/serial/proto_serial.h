#pragma once

#include "serial_base.h"

#include <google/protobuf/message.h>
#include <google/protobuf/repeated_field.h>


namespace NPb = google::protobuf;;


namespace NDnn {

	template <typename T>
    class IProtoSerial;
    class IMetaProtoSerial;

	class TProtoSerial: public TSerialBase {
    public:
        TProtoSerial(NPb::Message& message, ESerialMode mode);

        bool operator() (ui32& v, int protoField);

        bool operator() (double& v, int protoField);

        bool operator() (bool& v, int protoField);

        bool operator() (TString& v, int protoField);

    #ifdef LA_SYSTEM
        bool operator() (TVector<double>& v, int protoField);

        bool operator() (TVectorD& v, int protoField);

        bool operator() (TMatrixD& m, int protoField);
    #endif

        bool operator() (NPb::Message& m, int protoField);

        template <typename T>
        bool operator() (TVector<T>& v, int protoField);

        template <typename T>
        bool operator() (IProtoSerial<T>& v, int protoField);

        bool operator() (IMetaProtoSerial& v, int protoField, bool newMessage = false);

        template <typename T>
        bool operator() (T& v) {
            return (*this)(v, CurrentFieldNumber++);
        }

        bool operator() (IMetaProtoSerial& v) {
            return (*this)(v, CurrentFieldNumber);
        }
        
        void DuplicateSingleRepeated(int protoField, ui32 size, bool allow_zero = true);

        const NPb::FieldDescriptor* GetFieldDescr(int protoField);

    private:
        template <typename T>
        T* GetMutMessage(int protoField, bool newMessage = false) {
            auto* fd = GetFieldDescr(protoField);
            NPb::Message* message;
            if (fd->is_repeated()) {
                auto* rfPtr = Refl->MutableRepeatedPtrField<T>(&Message, fd);
                if ((rfPtr->size() == 0) || ((newMessage) && (IsOutput()))) {
                    message = Refl->AddMessage(&Message, fd);
                } else {
                    message = rfPtr->Mutable(0);
                }
            } else {
                ENSURE(!newMessage, "Trying to create new message on non repeated field");
                message = Refl->MutableMessage(&Message, fd);
            }
            T *m = dynamic_cast<T*>(message);
            ENSURE(m, "Failed to serialize field while casting " << protoField);
            return m;
        }

        template <typename T>
        const T* GetMessage(int protoField) {
            const NPb::Message* message = &Refl->GetMessage(Message, GetFieldDescr(protoField));
            const T *m = dynamic_cast<const T*>(message);
            ENSURE(m, "Failed to serialize field while casting " << protoField);
            return m;
        }

        bool HasField(int protoField);

    private:
        const NPb::Reflection* Refl;
        const NPb::Descriptor* Descr;

        NPb::Message& Message;

        int CurrentFieldNumber;
    };

	template <typename T>
    class IProtoSerial {
    public:
        using TProto = T;

        TProto Serialize();

        void Deserialize(TProto& proto);

        virtual void SerialProcess(TProtoSerial& serial) = 0;
    };




} // namespace NDnn

#include "proto_serial-inl.h"