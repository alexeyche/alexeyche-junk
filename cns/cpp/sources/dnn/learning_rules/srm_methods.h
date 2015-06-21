#pragma once

#include <snnlib/util/fastapprox/fastlog.h>

class SRMMethods {
public:
    static inline double LLH(SpikeNeuronBase *n) {
		return LLH_formula(n->fired(), n->getFiringProbability());
    }

    static inline double LLH_given_Y(SpikeNeuronBase *n, const double &fired) {
		return LLH_formula(fired, n->getFiringProbability());
    }

    static inline double dLLH_dw(SpikeNeuronBase *n, Synapse *syn) {
    	return dLLH_dw_formula(n->getFiringProbability(), n->act_rt.probDeriv(n->y), n->M, n->fired, syn->x);
    }

    static inline double dLLH_dw_given_Y(SpikeNeuronBase *n, Synapse *syn, const double &fired) {
		return dLLH_dw_formula(n->p, n->act_rt.probDeriv(n->y), n->M, fired, syn->x);
    }
private:
	static inline double LLH_formula(const double &fired, const double &p) {
		return fired*log(p) + (1 - fired) * log(1-p);
	}
	static inline double dLLH_dw_formula(const double &p, const double &p_stroke, const double &M, const double &fired, const double &x) {
		return (p_stroke/(p/M)) * (fired - p) * fabs(x);
	}
};
