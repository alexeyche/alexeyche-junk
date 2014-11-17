#pragma once

#include <snnlib/base.h>


class TuningCurve : public Obj {
public:
	virtual void init(const ConstObj *_c) = 0;
	virtual double calculateResponse(const double &I) = 0;
};

class BlankTuningCurve : public TuningCurve {
public:
	void init(const ConstObj *_c) {}
	double calculateResponse(const double &I) { return 0.0; }
};

