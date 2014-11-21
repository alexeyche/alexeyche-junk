#pragma once

#include <snnlib/core.h>

typedef pair<size_t, size_t> NeuronAddress;


class SimNeuron {
public:
	SimNeuron(Neuron *_n, size_t l_id, size_t n_id) : na(l_id, n_id), n(_n) {

    }
    NeuronAddress na;
    Neuron *n;
};
