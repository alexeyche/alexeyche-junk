#ifndef CONNECTIONS_H
#define CONNECTIONS_H


#include "groups.h"


namespace srm {
    void connectFeedForward(NeuronGroup *gr, Neuron *n, vec weights) {
        if(gr->group.size() != weights.n_elem) {
            throw SrmException("Can't connect: Weights vector size do no equal to NeuronGroup size\n");
        }
        for(size_t gi=0; gi<gr->group.size(); gi++) {
            n->add_input(gr->group[gi], weights(gi));
        }
    }
    void connectFeedForward(NeuronGroup *gr, Neuron *n, double start_weight) {
        vec w(gr->group.size());
        w.fill(start_weight);
        connectFeedForward(gr, n, w);
    }

};


#endif
