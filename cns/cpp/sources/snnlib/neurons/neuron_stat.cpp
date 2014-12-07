
#include <snnlib/protos/stat.pb.h>

#include "neuron_stat.h"
#include "neuron.h"

NeuronStat::NeuronStat(Neuron *n) : Serializable(ENeuronStat) {
    if(n) {
        for(size_t syn_i=0; syn_i<n->syns.size(); syn_i++) {
            syns.push_back(vector<double>());
            w.push_back(vector<double>());
        }
    }
}


void NeuronStat::collect(Neuron *n) {
    if(p.size()>STAT_COLLECT_LIMIT) return;
    p.push_back(n->p);
    u.push_back(n->y);
    for(size_t syn_i=0; syn_i<n->syns.size(); syn_i++) {
        syns[syn_i].push_back(n->syns[syn_i]->x);
        w[syn_i].push_back(n->syns[syn_i]->w);
    }

}

ProtoPack NeuronStat::serialize() {
    Protos::NeuronStat *stat = getNewMessage();
    for(auto it=p.begin(); it != p.end(); ++it) {
        stat->add_p(*it);
    }
    for(auto it=u.begin(); it != u.end(); ++it) {
        stat->add_u(*it);
    }
    assert(syns.size() == w.size());
    for(size_t syn_i=0; syn_i < syns.size(); syn_i++) {
        Protos::NeuronStat::SynStat* syn_stat = stat->add_syns();
        for(size_t el_i=0; el_i<syns[syn_i].size(); el_i++) {
            syn_stat->add_x(syns[syn_i][el_i]);
            syn_stat->add_w(w[syn_i][el_i]);
        }
    }
    return ProtoPack({stat});
}

