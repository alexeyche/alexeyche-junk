#ifndef AXON_H
#define AXON_H


#include <sim/core.h>
#include <sim/util/rand/common_rand_gen.h>

using namespace sim;

#include "basic_elements.h"

#define DEFAULT_MAX_D 20

struct AxonDelayOptions {
	AxonDelayOptions(int n, RandGen* generator_c = NULL): n(n), generator(generator_c) {
		if(!generator) {
			generator = new UnifRandGen(1,DEFAULT_MAX_D);
		}			
	}
	~AxonDelayOptions() { delete generator; }
	RandGen* setGenerator(RandGen* gen) {
		delete generator;
		generator = gen;
	}
	RandGen* getGenerator() {
		return generator;
	}
	int n;
private:	
	RandGen* generator;		
};

class AxonDelay {
public:	
	AxonDelay(int n) : n(n), I_in(n), I_out(n), delays(n), filled(0), delays_cur(n) {
		delays.fill(0);
	}
	void setInput(vec in) {

	}
	vec getOutput() {

	}
	void computeMe(double dt) {
		uvec active_axons = find(I_in > 0);		
		delays_cur(active_axons) += dt;		
		uvec transmited_ind = find(delays_cur-delays >= 0);
		if(transmited_ind.n_elem > 0) {
			I_out(transmited_ind) = I_in(transmited_ind);
			delays_cur(transmited_ind).fill(0);
		}
	}
	void prepareMe(AxonDelayOptions *ado) {						
		delays(span(filled, filled+ado->n)) = ado->getGenerator()->gen(ado->n);
		filled += ado->n;		
	}

	vec I_in;
	vec I_out;
	vec delays;
	int n;
private:
	int filled;
	vec delays_cur;
	
};

#endif