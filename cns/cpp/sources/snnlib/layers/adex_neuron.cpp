
#include "adex_neuron.h"



void AdExNeuronStat::collect(AdExNeuron *n) {
    ns->collect(n);
    a.push_back(n->a);
}