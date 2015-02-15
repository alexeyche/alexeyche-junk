#pragma once

#include <dnn/io/stream.h>

namespace dnn {


template <typename T>
class Serializable {
public: 
    virtual void in(IStream &in) const = 0;
    virtual void out(OStream &in) const = 0;
    const string name() const {
    	return string("123");
    }
    
};


}