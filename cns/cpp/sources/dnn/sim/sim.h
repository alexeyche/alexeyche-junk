#pragma once

#include <dnn/base/factory.h>
#include <dnn/util/barrier.h>
#include "builder.h"

namespace dnn {

class Sim {
public:
	Sim(const Constants &_c) : c(_c), duration(0.0) {
	}
	void build(Stream* input_stream = nullptr) {
		Builder b(c);
		if(input_stream) {
			b.setInputModelStream(input_stream);
		}
		neurons = b.buildNeurons();			
		for(auto &n: neurons) {
			if(n.ref().inputIsSet()) {
				duration = std::max(duration, n.ref().getInput().getDuration());
			}
		}
	}
	
	void serialize(Stream &output_stream) {
		for(auto &n : neurons) {
			output_stream.writeObject(n.ptr());
		}
	}

	
	
	static void runWorker(Sim &s, size_t from, size_t to, size_t jobs) {
		cout << "Hello from " << from << " : " << to << "\n";
	}
	void run(size_t jobs) {
		vector<IndexSlice> slices = dispatchOnThreads(neurons.size(), jobs);
		vector<std::thread> threads;
		
		for(auto &slice: slices) {
			threads.emplace_back(Sim::runWorker, std::ref(*this), slice.from, slice.to, jobs);
		}
		for(auto &t: threads) {
			t.join();
		}
	}


private:
	double duration;
	const Constants &c;
	vector<InterfacedPtr<SpikeNeuronBase>> neurons;
};

}
