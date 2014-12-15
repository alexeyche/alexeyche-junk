#pragma once

#include <snnlib/core.h>

typedef pair<size_t, size_t> NeuronAddress;


class SimNeuron {
public:
	SimNeuron(Layer *_l, Neuron *_n, size_t l_id, size_t n_id) : na(l_id, n_id), n(_n), l(_l) {
        _n->provideRuntime(n_rt);
    }
    NeuronRuntime n_rt;
    NeuronAddress na;
    Neuron *n;
    Layer *l;
};
