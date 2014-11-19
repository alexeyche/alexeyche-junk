
#include "adex_neuron.h"



void AdExNeuronStat::collect(AdExNeuron *n) {
    ns->collect(n);
    a.push_back(n->a);
}
AdExNeuronStat::AdExNeuronStat(const AdExNeuronStat &another) : Serializable(EAdExNeuronStat), a(another.a) {
    copyFrom(another);
    ns = new NeuronStat(*another.ns);
}
Protos::AdExNeuronStat *AdExNeuronStat::serialize() {
    Protos::AdExNeuronStat *stat = getNew();
    Protos::NeuronStat *nstat = ns->serialize();
    stat->set_allocated_stat(new Protos::NeuronStat(*nstat));
    for(auto it=a.begin(); it != a.end(); ++it) {
        stat->add_a(*it);
    }
    return stat;

}
