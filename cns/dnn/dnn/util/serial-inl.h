#pragma once

#include <dnn/util/log/log.h>
#include <dnn/util/optional.h>

namespace NDnn {

    template <typename T>
    void TProtoSerial::operator() (IProtoSerial<T>& v, int protoField, bool createNewMessage) {
        if (IsOutput()) {
            typename IProtoSerial<T>::TProto* mess;

            if (!createNewMessage) {
                mess = GetEmbedMutMessage<typename IProtoSerial<T>::TProto>(protoField);
            } else {
                mess = new typename IProtoSerial<T>::TProto();
            }
            L_DEBUG << "Serial process of IProtoSerial with type " << mess->GetTypeName() << " as " << protoField << " field number in " << Message.GetTypeName() << (createNewMessage ? "" : ", preserving old message");    
            TProtoSerial serial(*mess, Mode);
            v.SerialProcess(serial);

            if (createNewMessage) {
                (*this)(*mess, protoField);
                delete mess;
            }
        } else 
        if (IsInput()) {
            typename IProtoSerial<T>::TProto mess;
            (*this)(mess, protoField);

            TProtoSerial serial(mess, Mode);
            v.SerialProcess(serial);
            
            // if (!createNewMessage) {

            //     auto* fd = GetFieldDescr(protoField);
            //     if (fd->is_repeated()) {
            //         L_DEBUG << "Deleting top repeated message";
            //         Refl->MutableRepeatedPtrField<NPb::Message>(&Message, fd)->DeleteSubrange(0, 1);
            //     }
            // }

        }
        

        
        
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