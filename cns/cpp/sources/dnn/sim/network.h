#pragma once

#include <dnn/util/pretty_print.h>
#include <dnn/util/spikes_list.h>

namespace dnn {

class Conn : public Printable {
public:
	Conn(size_t _n_id, size_t _syn_id) : n_id(_n_id), syn_id(_syn_id) {}
	
	size_t n_id;
	size_t syn_id;
	
	void print(std::ostream& str) const {
		str << "(" << n_id << ":" << syn_id << ")";
	}
};

class Network : public Printable {
public:
	Network(vector<InterfacedPtr<SpikeNeuronBase>>& neurons) : spikes_list(neurons.size()) {
		conn_map.resize(neurons.size());
		for(auto &n: neurons) {
			vector<InterfacedPtr<SynapseBase>>& syns = n.ref().getSynapses();
            for(size_t con_i=0; con_i <syns.size(); con_i++) {
                conn_map[ syns[con_i].ref().getIdPre() ].push_back(
                	Conn(n.ref().id(), con_i)
                );
            }
        }
	}
	
	void print(std::ostream &str) const {
		str << "Network: \n";
		str << "\tConnMap: \n";
		for(size_t i=0; i<conn_map.size(); ++i) {
			cout << "neuron " << i << " cause spike in: ";
			cout << conn_map[i];
			cout << "\n";
		}	
	}
private:
	//vector<SpikesQueue> net_queues;

	vector<vector<Conn>> conn_map;
	SpikesList spikes_list;
	
};





}