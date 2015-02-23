#pragma once

#include "spike_neuron.h"


/*@GENERATE_PROTO@*/
struct LeakyIntegrateAndFireC : public Constants {
	double R;
	double C;
};


/*@GENERATE_PROTO@*/
struct LeakyIntegrateAndFireState : public SpikeNeuronState {
	double p;
	double u;
};


class LeakyIntegrateAndFire : public SpikeNeuron<LeakyIntegrateAndFireC, LeakyIntegrateAndFireState> {
public:

};