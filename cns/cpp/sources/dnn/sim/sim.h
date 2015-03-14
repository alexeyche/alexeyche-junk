#pragma once

#include <dnn/base/factory.h>
#include "builder.h"

namespace dnn {

class Sim {
public:
	Sim(const Constants &_c) : c(_c) {
	}
	void build(Stream* input_stream = nullptr) {
		Builder b(c);
		if(input_stream) {
			b.setInputModelStream(input_stream);
		}


		neurons = b.buildNeurons();
		
	}

private:
	const Constants &c;
	vector<InterfacedPtr<SpikeNeuronBase>> neurons;
};

}
