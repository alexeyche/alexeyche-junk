#pragma once

#include <snnlib/base.h>
#include <snnlib/serialize/serialize.h>

class Neuron;

class LearningRule : public Obj {
public:
	virtual void init(const ConstObj *_c, Neuron *_n) = 0;
    virtual void calculateWeightsDynamics() = 0;
    virtual void getStat(SerialPack &p) = 0;
};

class BlankLearningRule: public LearningRule {
public:
	void init(const ConstObj *_c, Neuron *_n) {}
    void calculateWeightsDynamics() {}
    void getStat(SerialPack &p) {  }
};
