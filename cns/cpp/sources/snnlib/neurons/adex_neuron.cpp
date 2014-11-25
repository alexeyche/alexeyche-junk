
#include "adex_neuron.h"



void AdExNeuronStat::collect(AdExNeuron *n) {
    if(a.size()>STAT_COLLECT_LIMIT) return;
    a.push_back(n->a);
}
AdExNeuronStat::AdExNeuronStat(const AdExNeuronStat &another) : Serializable(EAdExNeuronStat), a(another.a) {
    copyFrom(another);
}
Protos::AdExNeuronStat *AdExNeuronStat::serialize() {
    Protos::AdExNeuronStat *stat = getNew();
    for(auto it=a.begin(); it != a.end(); ++it) {
        stat->add_a(*it);
    }
    return stat;

}
