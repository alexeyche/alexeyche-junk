#pragma once

#include <google/protobuf/message.h>
#include <dnn/base/base.h>

namespace NDnn {

    template <typename T>
    class ISerial;

    class TSerializer {
    public:
        enum class ESerialMode {
            IN = 0,
            OUT = 1
        };

        TSerializer(google::protobuf::Message& message, ESerialMode mode);

        void operator() (ui32& v, int protoField);

        void operator() (TString& v, int protoField);

        // void operator() (TVector<double>& v, int protoField);

        template <typename T>
        void operator() (ISerial<T>& v, int protoField);

        // void operator() (TVectorD& v, int protoField);

        // void operator() (TMatrixD& m, int protoField);

        void operator() (google::protobuf::Message& m, int protoField);

        bool IsInput() const;

        bool IsOutput() const;

    private:
        template <typename T>
        T* GetMutMessage(int protoField) {
            google::protobuf::Message* message = Refl->MutableMessage(&Message, Descr->FindFieldByNumber(protoField));
            T *m = dynamic_cast<T*>(message);
            ENSURE(m, "Failed to serialize field " << protoField);
            return m;
        }

        template <typename T>
        const T* GetMessage(int protoField) {
            const google::protobuf::Message* message = &Refl->GetMessage(Message, Descr->FindFieldByNumber(protoField));
            const T *m = dynamic_cast<const T*>(message);
            ENSURE(m, "Failed to serialize field " << protoField);
            return m;
        }

    private:
        const google::protobuf::Reflection* Refl;
        const google::protobuf::Descriptor* Descr;

        google::protobuf::Message& Message;

        ESerialMode Mode;
    };


    template <typename T>
    class ISerial {
    public:
        using TProto = T;

        TProto Serialize();

        void Deserialize(TProto& proto);

        virtual void SerialProcess(TSerializer& serial) = 0;
    };

} // namespace NDnn

#include "serial-inl.h"