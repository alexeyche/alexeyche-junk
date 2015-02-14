#pragma once

#include "input.h"

namespace dnn {

struct TSInputConstants {};

struct TSInputState {};


class TSInput : public Input<TSInputConstants, TSInputState> {
public:
	typedef Input<TSInputConstants, TSInputState> Parent;
	TSInput(TSInputConstants _c) : Parent(_c) {}
	double evaluate() {
		return 0.0;
	}
};



}