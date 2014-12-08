#pragma once

#include <snnlib/util/fastapprox/fastlog.h>

class SRMMethods {
public:
    static inline double LLH(Neuron *n) {
		//cout <<  "p == " << n->p << " LLH: " << n->fired << "*" <<  log(n->p+0.001) << " + (1 - " << n->fired << ")* " << log(1-n->p-0.001) << "\n";
		return LLH_formula(n->fired, n->p);
    }

    static inline double LLH_given_Y(Neuron *n, const double &fired) {
		return LLH_formula(fired, n->p);
    }

    static inline double dLLH_dw(Neuron *n, Synapse *syn) {
    	return dLLH_dw_formula(n->p, n->act->probDeriv(n->y), n->M, n->fired, syn->x);
    }

    static inline double dLLH_dw_given_Y(Neuron *n, Synapse *syn, const double &fired) {
		return dLLH_dw_formula(n->p, n->act->probDeriv(n->y), n->M, fired, syn->x);
    }
private:
	static inline double LLH_formula(const double &fired, const double &p) {
		return fired*fastlog(p+0.001) + (1 - fired) * fastlog(1-p-0.001);
	}
	static inline double dLLH_dw_formula(const double &p, const double &p_stroke, const double &M, const double &fired, const double &x) {
		return (p_stroke/(p/M)) * (fired - p) * x;
	}
};
