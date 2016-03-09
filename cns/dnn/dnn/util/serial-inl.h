#pragma once

namespace NDnn {

    template <typename T>
    void TSerializer::operator() (ISerial<T>& v, int protoField) {
        typename ISerial<T>::TProto* mess = GetMutMessage<typename ISerial<T>::TProto>(protoField);
        TSerializer serial(*mess, Mode);
        v.SerialProcess(serial);
    }
        // switch (Mode) {
        //     case ESerialMode::IN:
        //     {
        //         typename ISerial<T>::TProto* mess = GetMutMessage<typename ISerial<T>::TProto>(protoField);
        //         TSerializer serial(*mess, ESerialMode::IN);
        //         v.SerialProcess(serial);
        //     }
        //     break;
        //     case ESerialMode::OUT:
        //     {


        //     }
        //     break;
        // }

    template <typename T>
    typename ISerial<T>::TProto ISerial<T>::Serialize() {
        TProto mess;
        TSerializer serial(mess, TSerializer::ESerialMode::OUT);
        SerialProcess(serial);
        return mess;
    }

    template <typename T>
    void ISerial<T>::Deserialize(typename ISerial<T>::TProto& proto) {
        TSerializer serial(proto, TSerializer::ESerialMode::IN);
        SerialProcess(serial);
    }

} // namespace NDnn