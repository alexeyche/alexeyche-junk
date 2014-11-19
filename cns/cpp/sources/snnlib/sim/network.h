#pragma once

#include <snnlib/util/spikes_list.h>

typedef priority_queue<SynSpike, vector<SynSpike>, CompareSynSpike> SpikeQueue;


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
	Network() {
		conn_map = nullptr;
		input_queues = nullptr;
		net_queues = nullptr;
	}
	Network(const Sim *s) {
		init(s);
	}
	void init(const Sim *s);
	void configureConnMap(const Sim *s);
	void propagateSpike(const size_t &global_id, const double &t);
	void dispathSpikes(const SpikesList &sl);

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
		str << "net_queue: \n";
		for(size_t i=0; i<total_size; i++) {
			SpikeQueue net_queue_copy(net_queues[i]);
			str << "queue of " << i << ": ";
			for(size_t el_i=0; el_i < net_queue_copy.size(); el_i++) {
				str << net_queue_copy.top() << ", ";
				net_queue_copy.pop();
			}
			str << "\n";
		}



	}

	const Sim *s;
	size_t total_size;

	SpikeQueue *input_queues;
	SpikeQueue *net_queues;
	vector<Conn> *conn_map;
	SpikesList spikes_list;
};
