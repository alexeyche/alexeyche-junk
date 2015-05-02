#pragma once


#include <dnn/io/stream.h>
#include <dnn/util/spinning_barrier.h>
#include <dnn/neurons/spike_neuron.h>
#include <dnn/base/constants.h>

#include "builder.h"
#include "network.h"

namespace dnn {

class Sim : public Printable {
public:
	Sim(const Constants &_c) : c(_c), duration(0.0) {}
	
	void build(Stream* input_stream = nullptr) {
		Builder b(c);
		if(input_stream) {
			b.setInputModelStream(input_stream);
		}
		neurons = b.buildNeurons();			
		for(auto &n: neurons) {
			duration = std::max(duration, n.ref().getSimDuration());
		}
		net = uptr<Network>(new Network(neurons));
	}
	
	void serialize(Stream &output_stream) {
		for(auto &n : neurons) {
			output_stream.writeObject(n.ptr());
		}
	}
	void saveStat(Stream &str) {
		for(auto &n: neurons) {
			if(n.ref().getStat().on()) {
				Statistics st = n.ref().getStat();
				str.writeObject(&st); 
			}
		}
	}
	void saveSpikes(Stream &str) {
		if(!net.get()) {
			throw dnnException()<< "Sim network was not found. You need to build sim\n";
		}
		str.writeObject(&net->spikesList());
	}	
	
	static void runWorkerRoutine(Sim &s, size_t from, size_t to, SpinningBarrier &barrier) {
		Time t(s.c.sim_conf.dt);

		for(size_t i=from; i<to; ++i) {				
			s.neurons[i].ref().resetInternal();
		}
		barrier.wait();
		for(; t<s.duration; ++t) {
			for(size_t i=from; i<to; ++i) {				
				s.neurons[i].ref().calculateDynamicsInternal(t);
				
				if(s.neurons[i].ref().fired()) {
					//cout << "Spiked " << s.neurons[i].ref().id() << " at " << t.t << "\n";					
					s.net->propagateSpike(s.neurons[i].ref(), t.t);
					s.neurons[i].ref().setFired(false);
				}
			}
			barrier.wait();
		}
		barrier.wait();
	}

	static void runWorker(Sim &s, size_t from, size_t to, SpinningBarrier &barrier, std::exception_ptr &eptr) {
		try {
			runWorkerRoutine(s, from, to, barrier);		
		} catch (const dnnException &e) {
			eptr = std::current_exception();
			barrier.fail();
		} catch (const dnnInterrupt &e) {
			// pass
		}
	}
	void run(size_t jobs) {
		if(fabs(duration) < 0.00001) {
			throw dnnException() << "Duration of simulation is " << duration << ". Check that input data was provided\n";
		}

		vector<IndexSlice> slices = dispatchOnThreads(neurons.size(), jobs);
		vector<std::thread> threads;
		vector<std::exception_ptr> exceptions;

		SpinningBarrier barrier(jobs);		
		for(auto &slice: slices) {
			exceptions.emplace_back();
			threads.emplace_back(
				Sim::runWorker, 
				std::ref(*this), 
				slice.from, 
				slice.to, 
				std::ref(barrier), 
				std::ref(exceptions.back())
			);
			
		}
		for(auto &t: threads) {
			t.join();
		}
		for(auto eptr: exceptions) {
			try {
				if(eptr) {
					std::rethrow_exception(eptr);
				}	
			} catch(const dnnException &dnn_e) {
				throw;
			}
		}
		
	}
	void setMaxDuration(const double Tmax) {
		if(fabs(duration) < 0.00001) {
			throw dnnException() << "Setting max duration for empty sim\n";
		}
		duration = min(Tmax, duration);
	}
	void print(std::ostream &str) const {
		str << "Sim\n";
		str << "\t" << neurons.size() << " ready to simulate for " << duration << "ms\n";
	}

protected:
	double duration;
	const Constants &c;
	vector<InterfacedPtr<SpikeNeuronBase>> neurons;
	uptr<Network> net;
};

}
