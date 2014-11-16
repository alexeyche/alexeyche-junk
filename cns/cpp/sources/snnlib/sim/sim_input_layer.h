#pragma once


#include <snnlib/core.h>

class SimInputLayer {
protected:
	SimInputLayer() {}
public:
	virtual void calculateDynamics(const size_t &ni, const double &x) = 0;
	SimInputLayer(size_t size) {
		init(size);
	}
    void init(size_t size) {
		
	}
	~SimInputLayer() {
		
	}

};