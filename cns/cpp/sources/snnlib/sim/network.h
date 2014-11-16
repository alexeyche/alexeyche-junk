#pragma once


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
	}
	Network(size_t net_size) {
		init(net_size);
	}
	void init(size_t net_size) {
		conn_map = new vector<Conn>[net_size];
		spikes_list = new vector<double>[net_size];
	}
	~Network() {
		if(conn_map) delete []conn_map;
		if(spikes_list) delete []spikes_list;
	}
	
	vector<Conn> *conn_map;	
	vector<double> *spikes_list;
};