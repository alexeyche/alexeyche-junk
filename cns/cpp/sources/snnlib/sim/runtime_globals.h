#pragma once

#include "reward_control.h"

class RuntimeGlobals {
public:
	RuntimeGlobals(RewardControl *_rc) : rc(_rc) {}
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
private:
	double dt;
    Constants *c;
    RewardControl *rc;
};
