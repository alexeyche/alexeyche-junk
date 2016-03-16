#pragma once

#include <dnn/util/log/log.h>
#include <dnn/util/optional.h>

namespace NDnn {

    #define CHECK_FIELD() \
        if ((!HasField(protoField)) && IsInput()) { \
            return false; \
        } \

    template <typename T>
    bool TProtoSerial::SerialRepeated(IProtoSerial<T>& v, int idx, int protoField) {
        CHECK_FIELD();

        typename IProtoSerial<T>::TProto mess;
        // L_DEBUG << "Proto " << (Mode == ESerialMode::IN ? "IN" : "OUT") << ": Serial process of repeated IProtoSerial with type " << mess.GetTypeName() << " as " << protoField << " field number in " << Message.GetTypeName();
        
        if (IsInput()) {
            SerialRepeated(mess, idx, protoField);
        }

        TProtoSerial serial(mess, Mode);
        v.SerialProcess(serial);
    
        if (IsOutput()) {
            SerialRepeated(mess, idx, protoField);
        }
        // L_DEBUG << "Proto " << (Mode == ESerialMode::IN ? "IN" : "OUT") << ": Serial process done";
        return true;
    }

    template <typename T>
    bool TProtoSerial::operator() (IProtoSerial<T>& v, int protoField) {
        CHECK_FIELD();

        typename IProtoSerial<T>::TProto mess;
        // L_DEBUG << "Proto " << (Mode == ESerialMode::IN ? "IN" : "OUT") << ": Serial process of IProtoSerial with type " << mess.GetTypeName() << " as " << protoField << " field number in " << Message.GetTypeName();
        
        if (IsInput()) {
            (*this)(mess, protoField);    
        }

        TProtoSerial serial(mess, Mode);
        v.SerialProcess(serial);
    
        if (IsOutput()) {
            (*this)(mess, protoField);
        }
        // L_DEBUG << "Proto " << (Mode == ESerialMode::IN ? "IN" : "OUT") << ": Serial process done";
        return true;
    }

    template <typename T>
    bool TProtoSerial::operator() (TVector<T>& v, int protoField) {
        CHECK_FIELD();
        if (IsInput()) {
            v.resize(Refl->FieldSize(Message, GetFieldDescr(protoField)));
        }
        for (int idx=0; idx < v.size(); ++idx) {
            SerialRepeated(v[idx], idx, protoField);
        }
        return true;
    }


    #undef CHECK_FIELD

    template <typename T>
    typename IProtoSerial<T>::TProto IProtoSerial<T>::Serialize() {
        TProto mess;
        TProtoSerial serial(mess, TProtoSerial::ESerialMode::OUT);
        SerialProcess(serial);
        return mess;
    }

    template <typename T>
    void IProtoSerial<T>::Deserialize(typename IProtoSerial<T>::TProto& proto) {
        TProtoSerial serial(proto, TProtoSerial::ESerialMode::IN);
        SerialProcess(serial);
    }

    template <typename T>
    void IProtoSerial<T>::Deserialize(const typename IProtoSerial<T>::TProto& proto) {
        Deserialize(const_cast<typename IProtoSerial<T>::TProto&>(proto));
    }

} // namespace NDnn