#pragma once

#include "proto_serial.h"

#include <dnn/util/log/log.h>


namespace NDnn {

	template <typename T>
    void TMetaProtoSerial::operator() (IProtoSerial<T>& v) {
    	L_DEBUG << "MetaProto " << (Mode == ESerialMode::IN ? "IN" : "OUT") << ": Serial process of message " << Message.GetTypeName() << " as " << DstFieldNumber;
        
        TProtoSerial serial(Message, Mode);
        
        v.SerialProcess(serial);

        L_DEBUG << "MetaProto " << (Mode == ESerialMode::IN ? "IN" : "OUT") << ": Serial process done";
    }

} // namespace Ndnn