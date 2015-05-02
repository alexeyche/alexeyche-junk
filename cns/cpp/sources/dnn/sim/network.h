#pragma once

#include <dnn/util/pretty_print.h>
#include <dnn/util/spikes_list.h>
#include <dnn/util/act_vector.h>

namespace dnn {

class Conn : public Printable {
public:
	Conn(SpikeNeuronBase &_neuron, size_t _syn_id) : neuron(_neuron), syn_id(_syn_id) {}
	
	SpikeNeuronBase &neuron;
	size_t syn_id;
	
	void print(std::ostream& str) const {
		str << "(" << neuron.id() << ":" << syn_id << ")";
	}
};

class Network : public Printable {
public:
	Network(vector<InterfacedPtr<SpikeNeuronBase>>& neurons) : spikes_list(neurons.size()) {
		conn_map.resize(neurons.size());
		for(auto &n: neurons) {
			ActVector<InterfacedPtr<SynapseBase>>& syns = n.ref().getSynapses();
            size_t con_i = 0;
            for(auto &s: syns) {
            	conn_map[ s.ref().idPre() ].push_back(
                	Conn(n.ref(), con_i)
                );
                ++con_i;
            }
        }
	}

	void propagateSpike(const SpikeNeuronBase& neuron, const double &t) {
	    spikes_list[neuron.id()].push_back(t);
	    if(((1000.0*((spikes_list[neuron.id()].size())/t))>300.0)&&(t>1000)) {
	        throw dnnException()<< "Rate limit exceeded: " << spikes_list[neuron.id()].size() << " spikes of neuron " << neuron.id() << " at " << t << "\n";
	    }
	    for(auto &conn : conn_map[neuron.id()]) {
	        conn.neuron.enqueueSpike(
	        	SynSpike(
	        		  neuron.id() /* source of spike */
	        		, conn.syn_id /* destination synapse */
	        		, t  + neuron.axonDelay() + conn.neuron.syns.get(conn.syn_id).ref().dendriteDelay() /* time of spike */
	        	)
	        );
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
	SpikesList& spikesList() {
		return spikes_list;
	}
private:
	vector<vector<Conn>> conn_map;
	SpikesList spikes_list;
	
};





}