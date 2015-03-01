#pragma once

#include "input.h"

#include <dnn/protos/generated.pb.h>
namespace dnn {

/*@GENERATE_PROTO@*/
struct TSInputC : public Serializable<Protos::TSInputC> {
    double dt;

    void serialize() {
        //acquire(str) << "dt: " << dt << Self::End;
    }
};

/*@GENERATE_PROTO@*/
struct TSInputState : public Serializable<Protos::TSInputState> {
    size_t index;

    void serialize() {
        //acquire(str) << "index: " << index << Self::End;
    }
};


class TSInput : public Input<TSInputC, TSInputState> {
public:
	typedef Input<TSInputC, TSInputState> Parent;

	double getValue() {
		return 0.0;
	}
};



}
