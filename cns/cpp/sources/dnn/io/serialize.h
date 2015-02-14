#pragma once

namespace dnn {


template <typename T>
class Serializable {
public: 
    virtual void in(IStream &in) = 0;
    virtual void out(OStream &in) = 0;

    
};


}