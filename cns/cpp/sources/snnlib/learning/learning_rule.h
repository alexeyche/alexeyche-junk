#pragma once

#include <snnlib/base.h>
#include <snnlib/serialize/serialize.h>

class Neuron;

class LearningRule : public Obj {
public:
	virtual void init(const ConstObj *_c, Neuron *_n) = 0;
    virtual void calculateWeightsDynamics() = 0;
    virtual void saveStat(SerialFamily &p) = 0;
    virtual void saveModel(SerialFamily &p) = 0;
protected:
	Neuron *n;    
};

class BlankLearningRule: public LearningRule {
public:
	void init(const ConstObj *_c, Neuron *_n) {
        n = _n;
	}
    void calculateWeightsDynamics() {}
    void saveStat(SerialFamily &p) {  }
    void saveModel(SerialFamily &p) {  }
};
