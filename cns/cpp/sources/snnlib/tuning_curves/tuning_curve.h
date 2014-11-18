#pragma once

#include <snnlib/base.h>


class TuningCurve : public Obj {
public:
	virtual void init(const ConstObj *_c, Neuron *_n) = 0;
	virtual void calculateResponse(const double &I) = 0;
};

class BlankTuningCurve : public TuningCurve {
public:
	void init(const ConstObj *_c, Neuron *_n) {}
	void calculateResponse(const double &I) { }
};

