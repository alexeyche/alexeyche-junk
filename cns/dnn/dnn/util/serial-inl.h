#pragma once

namespace NDnn {

    template <typename T>
    void TProtoSerial::operator() (IProtoSerial<T>& v, int protoField) {
        typename IProtoSerial<T>::TProto* mess = GetMutMessage<typename IProtoSerial<T>::TProto>(protoField);
        TProtoSerial serial(*mess, Mode);
        v.SerialProcess(serial);
    }

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

} // namespace NDnn