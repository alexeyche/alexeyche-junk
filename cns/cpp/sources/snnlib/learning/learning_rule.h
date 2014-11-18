#pragma once

#include <snnlib/base.h>
class Neuron;

class LearningRule : public Obj {
public:
	virtual void init(const ConstObj *_c, Neuron *_n) = 0;
};

class BlankLearningRule: public LearningRule {
public:
	void init(const ConstObj *_c, Neuron *_n) {}
};
