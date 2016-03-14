#include "util.h"

namespace NDnn {

    ui32 GetRepeatedFieldSizeFromMessage(const NPb::Message& message, int protoField) {
        const NPb::Reflection* refl = message.GetReflection();
        const NPb::Descriptor* descr = message.GetDescriptor();
        const auto* fd = descr->FindFieldByNumber(protoField);
        return refl->FieldSize(message, fd);
    }

} // namespace NDnn