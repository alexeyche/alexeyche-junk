#pragma once

#include <google/protobuf/message.h>
#include <dnn/base/base.h>

#include <iostream>

namespace NDnn {

    template <typename T>
    class IProtoSerial;

    class TSerialBase {
    public:
        enum class ESerialMode {
            IN = 0,
            OUT = 1
        };

        TSerialBase(ESerialMode mode);

        bool IsInput() const;

        bool IsOutput() const;

    protected:
        ESerialMode Mode;
    };

    class TProtoSerial: public TSerialBase {
    public:
        TProtoSerial(google::protobuf::Message& message, ESerialMode mode);

        void operator() (ui32& v, int protoField);

        void operator() (double& v, int protoField);

        void operator() (bool& v, int protoField);

        void operator() (TString& v, int protoField);

        // void operator() (TVector<double>& v, int protoField);

        // void operator() (TVectorD& v, int protoField);

        // void operator() (TMatrixD& m, int protoField);

        void operator() (google::protobuf::Message& m, int protoField);

        template <typename T>
        void operator() (IProtoSerial<T>& v, int protoField);

        template <typename T>
        void operator() (T& v) {
            (*this)(v, CurrentFieldNumber++);
        }

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

    class TSerialStream;

    class ISerialStream {
    public:
        virtual void SerialProcess(TSerialStream& serial) = 0;
    };


    class TSerialStream: public TSerialBase {
    public:
        TSerialStream(std::ostream& ostr);
        TSerialStream(std::istream& istr);

        template <typename T>
        void operator() (IProtoSerial<T>& v) {
            if (IsInput()) {
                TString s;
                (*IStr) >> s;
                T proto;
                DeserializeProtoFromString(s, proto);
                v.Deserialize(proto);
            } else {
                T proto = v.Serialize();
                TString s;
                SerializeProtoToString(proto, s);
                (*OStr) << s;
            }
        }

        template <typename T>
        void operator() (const IProtoSerial<T>& v) {
            (*this)(const_cast<IProtoSerial<T>&>(v));
        }
        virtual void operator() (ISerialStream& v) = 0;

        virtual void DeserializeProtoFromString(const TString& s, google::protobuf::Message& message) = 0;
        virtual void SerializeProtoToString(const google::protobuf::Message& message, TString& s) = 0;

    protected:
        std::ostream* OStr;
        std::istream* IStr;
    };



    class TSerialStreamProtoBin: public TSerialStream {
    public:
        TSerialStreamProtoBin(std::ostream& ostr);
        TSerialStreamProtoBin(std::istream& istr);

        void operator() (ISerialStream& v) override final;
        
        void DeserializeProtoFromString(const TString& s, google::protobuf::Message& message) override final;
        void SerializeProtoToString(const google::protobuf::Message& message, TString& s) override final;
    };

    class TSerialStreamProtoTxt: public TSerialStream {
    public:
        TSerialStreamProtoTxt(std::ostream& ostr);
        TSerialStreamProtoTxt(std::istream& istr);

        void operator() (ISerialStream& v) override final;

        void DeserializeProtoFromString(const TString& s, google::protobuf::Message& message) override final;
        void SerializeProtoToString(const google::protobuf::Message& message, TString& s) override final;
    };


} // namespace NDnn

#include "serial-inl.h"