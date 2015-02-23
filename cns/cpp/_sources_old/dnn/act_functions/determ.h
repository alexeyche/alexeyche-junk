#pragma once


#include "act_function.h"

#include <dnn/protos/generated.pb.h>

namespace dnn {

/*@GENERATE_PROTO@*/
struct DetermConstants : public Serializable<Protos::DetermConstants> {
    double threshold;

    void processStream(Stream &str) {
        acquire(str) << "threshold: " << threshold << Self::End;
    }
};


class Determ : public ActFunction<DetermConstants, bool> {
public:
	typedef ActFunction<DetermConstants, bool> Parent;
	Determ() {}

	bool evaluate() {
		return 1.0;
	}

};

}
