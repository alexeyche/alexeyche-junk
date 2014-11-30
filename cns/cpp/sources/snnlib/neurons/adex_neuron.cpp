
#include "adex_neuron.h"



void AdExNeuronStat::collect(AdExNeuron *n) {
    if(a.size()>STAT_COLLECT_LIMIT) return;
    NeuronStat::collect(n);
    a.push_back(n->a);
}

ProtoPack AdExNeuronStat::serialize() {
    ProtoPack p = NeuronStat::serialize();
    Protos::AdExNeuronStat *stat = getNewMessage<Protos::AdExNeuronStat>();
    for(auto it=a.begin(); it != a.end(); ++it) {
        stat->add_a(*it);
    }
    p.push_back(stat);
    return p;

}
