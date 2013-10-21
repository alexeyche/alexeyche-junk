#ifndef STAT_COLLECTOR_H
#define STAT_COLLECTOR_H


#include "neurons.h"

class StatCollector : public SimBasic {
public:	
	StatCollector(vec& val) : iter(0) { v = &val; } 
	void computeMe(double dt) {
		acc.col(iter) = *v;
		iter++;
	}
	void prepareMe(SimOptions so) {
		acc = zeros(v->n_elem, so.iterations);
	}	


	vec *v;
	mat acc;	
private:
	int iter;
};


#endif