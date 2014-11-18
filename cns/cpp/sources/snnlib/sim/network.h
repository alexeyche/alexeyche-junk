#pragma once

typedef priority_queue<SynSpike, vector<SynSpike>, CompareSynSpike> SpikeQueue;


struct Conn {
	size_t l_id;
	size_t n_id;
	size_t syn_id;
};

class Network {
public:
	Network() {
		conn_map = nullptr;
		spikes_list = nullptr;
		input_queues = nullptr;
		net_queues = nullptr;
	}
	Network(size_t input_size, size_t net_size) {
		init(input_size, net_size);
	}
	void init(size_t input_size, size_t _net_size) {
		total_size = input_size + _net_size;
		net_size = _net_size;
		conn_map = new vector<Conn>[total_size];
		spikes_list = new vector<double>[total_size];
		input_queues = new SpikeQueue[net_size];
		net_queues = new SpikeQueue[net_size];
	}
	void propagateSpike(const size_t &global_id, const double &t) {
		spikes_list[global_id].push_back(t);
	}
	~Network() {
		if(conn_map) delete []conn_map;
		if(spikes_list) delete []spikes_list;
		if(input_queues) delete []input_queues;
		if(net_queues) delete []net_queues;
	}
	
	size_t net_size;
	size_t total_size;

	SpikeQueue *input_queues;	
	SpikeQueue *net_queues;
	vector<Conn> *conn_map;	
	vector<double> *spikes_list;
};