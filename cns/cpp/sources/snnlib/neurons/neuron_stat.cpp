
#include <snnlib/protos/stat.pb.h>

#include "neuron_stat.h"
#include "neuron.h"

NeuronStat::NeuronStat(Neuron *n) : Serializable(ENeuronStat) {
    if(n) {
        for(size_t syn_i=0; syn_i<n->syns.size(); syn_i++) {
            syns.push_back(vector<double>());
        }
    }
}


void NeuronStat::collect(Neuron *n) {
    if(p.size()>STAT_COLLECT_LIMIT) return;
    cout << p.size() << "\n";
    p.push_back(n->p);
    u.push_back(n->y);
    for(size_t syn_i=0; syn_i<n->syns.size(); syn_i++) {
        syns[syn_i].push_back(n->syns[syn_i]->x);
    }

}

Protos::NeuronStat *NeuronStat::serialize() {
    Protos::NeuronStat *stat = getNew();
    for(auto it=p.begin(); it != p.end(); ++it) {
        stat->add_p(*it);
    }
    for(auto it=u.begin(); it != u.end(); ++it) {
        stat->add_u(*it);
    }
    for(auto it=syns.begin(); it != syns.end(); ++it) {
        Protos::NeuronStat::SynStat* syn_stat = stat->add_syns();
        for(auto it_val=it->begin(); it_val != it->end(); ++it_val) {
            syn_stat->add_x(*it_val);
        }
    }
    return stat;
}

