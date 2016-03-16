#pragma once

#include "proto_serial.h"

#include <dnn/util/log/log.h>


namespace NDnn {
	#define CHECK_FIELD() \
        if ((!HasField(protoField)) && IsInput()) { \
            return false;  \
        } \

	template <typename T>
    bool TMetaProtoSerial::operator() (IProtoSerial<T>& v) {
    	// L_DEBUG << "MetaProto " << (Mode == ESerialMode::IN ? "IN" : "OUT") << ": Serial process of message " << Message.GetTypeName() << " as " << DstFieldNumber;
        
        TProtoSerial serial(Message, Mode);
        
        v.SerialProcess(serial);

        // L_DEBUG << "MetaProto " << (Mode == ESerialMode::IN ? "IN" : "OUT") << ": Serial process done";
        return true;
    }

    #undef CHECK_FIELD
} // namespace Ndnn