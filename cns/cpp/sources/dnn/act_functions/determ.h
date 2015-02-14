#pragma once


#include "act_function.h"

namespace dnn {

struct DetermConstants {

};


class Determ : public ActFunction<DetermConstants, bool> {
public:
	typedef ActFunction<DetermConstants, bool> Parent;
	Determ(DetermConstants _c) : Parent(_c) {}

	bool evaluate() {
		return 1.0;
	}

};

}