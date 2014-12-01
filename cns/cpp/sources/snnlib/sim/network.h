#pragma once

#include <snnlib/util/spikes_list.h>
#include <snnlib/util/spikes_queue.h>

//typedef priority_queue<SynSpike, vector<SynSpike>, CompareSynSpike> SpikeQueue;


class Sim;

class Conn : public Printable {
public:
	size_t l_id;
	size_t n_id;
	size_t syn_id;
	void print(std::ostream& str) const {
		str << "(" << l_id << ":" << n_id << ":" << syn_id << ")";
	}
};

class Network : public Printable {
public:
	Network() : total_size(0) {
		conn_map = nullptr;
		input_queues = nullptr;
		net_queues = nullptr;
	}
	Network(const Sim *s) {
		init(s);
	}
	void init(const Sim *s);
	void configureConnMap();
	void propagateSpike(const size_t &global_id, const double &t);
	void dispathInputSpikes(const SpikesList &sl);

	void reset(bool reset_input_queue = false) {
		for(size_t global_id=0; global_id<total_size; ++global_id) {
			spikes_list[global_id].clear();
			net_queues[global_id].clear();
			if((reset_input_queue)&&(input_queues)) {
				input_queues[global_id].clear();
			}
		}
	}

	const SynSpike* getSpike(const size_t& global_id, const double &t);

	~Network() {
		if(conn_map) delete []conn_map;
		if(input_queues) delete []input_queues;
		if(net_queues) delete []net_queues;
	}
	void print(std::ostream& str) const {
		str << "Network: \n";
		str << "\tConnMap: \n";
		for(size_t i=0; i<total_size; i++) {
			cout << "neuron " << i << " cause spike in: ";
			print_vector<Conn>(conn_map[i], str, ",");
		}
		str << "input_queue: \n";
		for(size_t i=0; i<total_size; i++) {
			str << input_queues[i];
		}
		str << "spikes list: \n";
		str << spikes_list;


	}

	const Sim *s;
	size_t total_size;

	SpikesQueue *input_queues;
	SpikesQueue *net_queues;

	//SpikeQueue *input_queues;
	//SpikeQueue *net_queues;
	vector<Conn> *conn_map;
	SpikesList spikes_list;
};
