#pragma once

#include "input.h"

#include <dnn/protos/generated.pb.h>

namespace dnn {

/*@GENERATE_PROTO@*/
struct TSInputConstants : public Serializable<Protos::TSInputConstants> {
    double dt;

    void processStream(Stream &str) {
        acquire(str) << "dt: " << dt << Self::End;
    }
};

/*@GENERATE_PROTO@*/
struct TSInputState : public Serializable<Protos::TSInputState> {
    size_t index;

    void processStream(Stream &str) {
        acquire(str) << "index: " << index << Self::End;
    }
};


class TSInput : public Input<TSInputConstants, TSInputState> {
public:
	typedef Input<TSInputConstants, TSInputState> Parent;
	TSInput() {}
	double evaluate() {
		return 0.0;
	}
};



}
