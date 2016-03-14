#pragma once

#include <dnn/base/base.h>
#include <dnn/util/log/log.h>

#include <google/protobuf/message.h>

namespace NPb = google::protobuf;;


namespace NDnn {
	
	template <typename T>
    const T& GetRepeatedFieldFromMessage(const NPb::Message& message, int protoField, int index) {
        const NPb::Reflection* refl = message.GetReflection();
        const NPb::Descriptor* descr = message.GetDescriptor();
        const auto* fd = descr->FindFieldByNumber(protoField);
        const auto& messageField = refl->GetRepeatedMessage(message, fd, index);
        try {
            return dynamic_cast<const T&>(messageField);
        } catch (const std::bad_cast& error) {
            T m;
            L_ERROR << "Failed to cast " << messageField.GetTypeName() << " into " << m.GetTypeName();
            throw; 
        }
    }

    ui32 GetRepeatedFieldSizeFromMessage(const NPb::Message& message, int protoField);

} // namespace NDnn