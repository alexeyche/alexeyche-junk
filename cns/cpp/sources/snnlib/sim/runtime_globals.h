#pragma once

#include "reward_control.h"

class Network;

class RuntimeGlobals {
public:
	RuntimeGlobals(RewardControl *_rc, Network *_net) : rc(_rc), net(_net) {}
	void setDt(double _dt) {
		dt = _dt;
	}
	void setC(Constants &_c) {
		c = &_c;
	}

	inline const double& Dt() const {
		return dt;
	}
    inline const Constants& C() const {
        return *c;
    }
    inline Constants& mut_C() const {
        return *c;
    }
    inline void propagateReward(const size_t &ni, double rew) {
        rc->neuron_modulated_reward[ni] = rew;
    }
    inline const Reward* getReward(const size_t &ni) const {
    	return rc->neuron_prepared_reward[ni];
    }

    void initInputNeuronsFiringDelivery(Sim *s);

    void setInputNeuronsFiring(const size_t &last_layer_neuron_id, const double &t);

    inline const uchar inputNeuronsFiring(const size_t &last_layer_neuron_id) const {
        return input_neurons_firing[last_layer_neuron_id - last_layer_id_offset];
    }
    inline const bool doWeCareAboutInput() const {
        return input_spikes_iterators.size() != 0;
    }

    const size_t* current_class_id;
    const double& getSimTime() const {
        return sim_time;
    }
    void setSimTime(double t) {
        sim_time = t;
    }
    void incSimTime(double t) {
        sim_time += t;
    }
private:
    double sim_time;

	double dt;
    Constants *c;
    RewardControl *rc;
    Network *net;

    size_t last_layer_id_offset;
    vector<size_t> input_spikes_iterators;
    vector<uchar> input_neurons_firing;
};
