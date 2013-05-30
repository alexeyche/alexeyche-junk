#ifndef VOLTMETER_H
#define VOLTMETER_H


#include "neurons.h"

class VoltMeter : public SimBasic {
public:	
	VoltMeter(Neurons* el) : el(el) {}
	void computeMe(double dt) {
		acc.push_back(el->V);
	}

	Neurons* el;
	std::vector<vec> acc;	
};


#endif