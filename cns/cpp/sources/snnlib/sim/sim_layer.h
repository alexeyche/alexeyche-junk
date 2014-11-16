#pragma once


#include <snnlib/core.h>

class SimLayer {
protected:
	SimLayer() {}
public:
	virtual void calculate() = 0;
};