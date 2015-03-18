#pragma once

#include <dnn/util/pretty_print.h>
#include <dnn/util/spikes_list.h>

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
			vector<InterfacedPtr<SynapseBase>>& syns = n.ref().getSynapses();
            for(size_t con_i=0; con_i <syns.size(); con_i++) {
                conn_map[ syns[con_i].ref().getIdPre() ].push_back(
                	Conn(n.ref(), con_i)
                );
            }
        }
	}
	void propagateSpike(const SpikeNeuronBase& neuron, const double &t) {
	    spikes_list[neuron.id()].push_back(t);
	    if(((1000.0*((spikes_list[neuron.id()].size())/t))>300.0)&&(t>1000)) {
	        cerr << "Rate limit exceeded: " << spikes_list[neuron.id()].size() << " spikes of neuron " << neuron.id() << " at " << t << "\n";
	        terminate();
	    }
	    for(auto &conn : conn_map[neuron.id()]) {
	        conn.neuron.input_spikes.push(
	        	SynSpike(
	        		  neuron.id() /* source of spike */
	        		, conn.syn_id /* destination synapse */
	        		, t  + neuron.axon_delay + conn.neuron.syns[conn.syn_id].ref().dendrite_delay /* time of spike */
	        	)
	        );
	    }    
	    // for(size_t con_i=0; con_i < conn_map[neuron.id()].size(); con_i++) {
	    //     SynSpike sp;
	    //     sp.n_id = neuron.id();

	    //     sp.syn_id = conn_map[neuron.id()][con_i].syn_id;
	    //     SpikeNeuronBase &afferent_neuron = conn_map[neuron.id()][con_i].neuron; //s->layers[ conn_map[neuron.id()][con_i].l_id ]->neurons[ conn_map[neuron.id()][con_i].n_id ];
	      
	    //     sp.t = t  + neuron.axon_delay + afferent_neuron.syns[sp.syn_id].ref().dendrite_delay;

	    //     afferent_neuron.input_spikes.push(sp);
	    //     //net_queues[afferent_neuron.id()].asyncPush(sp);
	    //     // if(conn_map[global_id][con_i].l_id == 2) {
	    //     //     cout << "Propagating spike from " << global_id << " to " << glob_afferent_id << "\n";
	    //     //     cout << net_queues[glob_afferent_id] << "\n";
	    //     // }

    	// }
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
	vector<vector<Conn>> conn_map;
	SpikesList spikes_list;
	
};





}