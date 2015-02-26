#include "stream.h"

#include <dnn/io/serialize.h>

namespace dnn {

Stream& Stream::operator << (SerializableBase &v) {
    v.processStream(*this);
    return *this;
}



}


