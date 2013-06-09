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


class AxonDelay : public SimBasic {
public:	
	AxonDelay(int n, vec &in) : V_in(in), n(n), V_out(n), delays(n), filled(0), delays_cur(n), V_in_cur(n) {
		V_out.fill(0);
		delays.fill(0);
		delays_cur.fill(0);
		V_in_cur.fill(0);	
		cur_active_axons.fill(0);

	}
	vec& getOutput() {
		return V_out;
	}
	vec& getInput() {
		return V_in;
	}
	void computeMe(double dt) {		
		// get potentials
		uvec active_axons = find(V_in);
		 
		V_in_cur(active_axons) = V_in(active_axons);		
		V_in(active_axons).fill(0);

		// increment delays
		cur_active_axons = find(V_in_cur);		
		delays_cur(cur_active_axons) += dt;		
		// find finished axons
		uvec transmited_ind = find(delays_cur-delays >= 0);
		// transmit current
		if(transmited_ind.n_elem > 0) {
			V_out(transmited_ind) = V_in_cur(transmited_ind);
			V_in_cur(transmited_ind).fill(0);
			delays_cur(transmited_ind).fill(0);
		}
	}
	void prepareMe(AxonDelayOptions *ado) {						
		delays(span(filled, filled+ado->n)) = ado->getGenerator()->gen(ado->n);
		filled += ado->n;		
	}

	int n;
	vec &V_in;
	vec V_out;
	vec delays;	
	vec V_in_cur; // potentials need to deliver
	vec delays_cur;
private:
	int filled;
	
	
	//uvec active_axons;
	uvec cur_active_axons;	
};

#endif