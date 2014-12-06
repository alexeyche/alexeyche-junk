#pragma once



class SRMMethods {
public:	
    static inline double LLH(Neuron *n) {
		//cout <<  "p == " << n->p << " LLH: " << n->fired << "*" <<  log(n->p+0.001) << " + (1 - " << n->fired << ")* " << log(1-n->p-0.001) << "\n";
		return n->fired*log(n->p+0.001) + (1 - n->fired) * log(1-n->p-0.001);
    }
    static inline double dLLH_dw(Neuron *n, Synapse *syn) {
    	double p_stroke = n->act->probDeriv(n->y);
    	return p_stroke/(n->p/n->M) * (n->fired - n->p) * syn->x;
    }
};
