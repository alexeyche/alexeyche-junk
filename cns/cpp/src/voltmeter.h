#ifndef VOLTMETER_H
#define VOLTMETER_H


#include "neurons.h"

class VoltMeter : public SimBasic {
public:	
	VoltMeter(Neurons* el) : el(el), iter(0) {}
	void computeMe(double dt) {
		acc.col(iter) = el->V;
		iter++;
	}
	void prepareMe(SimOptions so) {
		acc = zeros(el->n, so.iterations);
	}	


	Neurons* el;
	mat acc;	
private:
	int iter;
};


#endif