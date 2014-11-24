

#include "network.h"

#include "sim.h"



void Network::init(const Sim *_s) {
    s = _s;

    total_size = s->input_neurons_count + s->net_neurons_count;

    conn_map = new vector<Conn>[total_size];
    spikes_list.init(total_size);

    //input_queues = new SpikeQueue[net_size];
    net_queues = new SpikesQueue[total_size];
    input_queues = new SpikesQueue[total_size];
}


void Network::configureConnMap() {
    for(size_t li=0; li<s->layers.size(); li++) {
        const Layer *l = s->layers[li];
        for(size_t ni=0; ni<l->N; ni++) {
            const Neuron* n = l->neurons[ni];
            for(size_t con_i=0; con_i < n->syns.size(); con_i++) {
                Conn conseq;
                conseq.l_id = li;
                conseq.n_id = ni;
                conseq.syn_id = con_i;

                conn_map[ n->syns[con_i]->id_pre ].push_back(conseq);
                //cout << "add conseq: " << n->syns[con_i]->id_pre << " -> " << conseq << "\n";
            }
        }
    }

}

void Network::propagateSpike(const size_t &global_id, const double &t) {
    spikes_list[global_id].push_back(t);

    const double &axon_delay = s->sim_neurons[global_id].n->axon_delay;
    for(size_t con_i=0; con_i < conn_map[global_id].size(); con_i++) {
        SynSpike sp;
        sp.n_id = global_id;

        sp.syn_id = conn_map[global_id][con_i].syn_id;
        Neuron *afferent_neuron = s->layers[ conn_map[global_id][con_i].l_id ]->neurons[ conn_map[global_id][con_i].n_id ];
        size_t glob_afferent_id = afferent_neuron->id;
        sp.t = t  + axon_delay + afferent_neuron->syns[sp.syn_id]->dendrite_delay;


        net_queues[glob_afferent_id].asyncPush(sp);
        // if(conn_map[global_id][con_i].l_id == 2) {
        //     cout << "Propagating spike from " << global_id << " to " << glob_afferent_id << "\n";
        //     cout << net_queues[glob_afferent_id] << "\n";
        // }

    }
}


const SynSpike* Network::getSpike(const size_t& global_id, const double &t) {
    if(const SynSpike* inp_sp = input_queues[global_id].getSpike(t)) {
        return inp_sp;
    }
    if(const SynSpike* net_sp = net_queues[global_id].getSpike(t)) {
        //cout << "Sending spike " << *net_sp << "\n";
        return net_sp;
    }
    return nullptr;
}

void Network::dispathInputSpikes(const SpikesList &sl) {
    if(sl.N != total_size) {
        cerr << "Can't dispatcher spikes list with different from network size\n";
        terminate();
    }
    vector<SynSpike> syn_spikes[sl.N];

    for(size_t ni=0; ni<sl.N; ni++) {
        for(size_t con_i=0; con_i < conn_map[ni].size(); con_i++) {
            for(size_t sp_i=0; sp_i<sl[ni].size(); sp_i++) {
                SynSpike sp;
                sp.n_id = ni;

                sp.syn_id = conn_map[ni][con_i].syn_id;
                Neuron *afferent_neuron = s->layers[ conn_map[ni][con_i].l_id ]->neurons[ conn_map[ni][con_i].n_id ];
                size_t glob_afferent_id = afferent_neuron->id;
                sp.t = sl[ni][sp_i] + afferent_neuron->syns[sp.syn_id]->dendrite_delay;
                syn_spikes[glob_afferent_id].push_back(sp);
            }
        }
    }
    for(size_t ni=0; ni<sl.N; ni++) {
        input_queues[ni].push_and_sort(syn_spikes[ni]);
    }

}
