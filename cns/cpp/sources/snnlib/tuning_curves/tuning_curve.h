#pragma once

#include <snnlib/base.h>


class TuningCurve : public Obj {
public:	
	virtual void init(size_t size, const ConstObj *_c) = 0;
	//virtual void calculateResponse(Neuron *n, const double &x) = 0;
};

class BlankTuningCurve : public TuningCurve {
public:	
	void init(size_t size, const ConstObj *_c) {}
	//void calculateResponse(Neuron *n, const double &x)  {}
};

