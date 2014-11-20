

#include "network.h"

#include "sim.h"


void Network::init(const Sim *_s) {
    s = _s;

    total_size = s->input_neurons_count + s->net_neurons_count;
    conn_map = new vector<Conn>[total_size];
    spikes_list.init(total_size);

    //input_queues = new SpikeQueue[net_size];
    net_queues = new SpikeQueue[total_size];

    configureConnMap(s);
}


void Network::configureConnMap(const Sim *s) {
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
}

void Network::dispathSpikes(const SpikesList &sl) {
    if(sl.N != total_size) {
        cerr << "Can't dispatcher spikes list with different from network size\n";
        terminate();
    }

    for(size_t ni=0; ni<sl.N; ni++) {
        for(size_t con_i=0; con_i < conn_map[ni].size(); con_i++) {
            for(size_t sp_i=0; sp_i<sl[ni].size(); sp_i++) {
                SynSpike *sp = new SynSpike();
                sp->n_id = ni;
                sp->syn_id = conn_map[ni][con_i].syn_id;
                Neuron *afferent_neuron = s->layers[ conn_map[ni][con_i].l_id ]->neurons[ conn_map[ni][con_i].n_id ];
                size_t glob_afferent_id = afferent_neuron->id;
                sp->t = sl[ni][sp_i] + afferent_neuron->syns[sp->syn_id]->dendrite_delay;
                net_queues[glob_afferent_id].push(shared_ptr<SynSpike>(sp));
            }
        }
    }
}
