#pragma once

#include <dnn/io/serialize.h>
#include "object.h"

namespace dnn {

template <typename Eval>
class DynamicObject : public Object {
public:
	virtual Eval evaluate() = 0;
};

template <typename Constants>
class StatelessObject : public SerializableBase  {
public:
    StatelessObject() {}
    void setConstants(Constants _c) {
        c = _c;
    }
    void processStream(Stream &str) {
        str << c ;
    }
protected:
    Constants c;
};


template <typename Constants, typename State>
class StateObject : public SerializableBase {
public:
    StateObject() {}
    void setConstants(Constants _c) {
        c = _c;
    }
    State& getState() {
        return s;
    }
    void processStream(Stream &str) {
        str << c << s;
    }
protected:
    Constants c;
    State s;
};

}
