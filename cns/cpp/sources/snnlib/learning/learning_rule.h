#pragma once

#include <snnlib/base.h>
#include <snnlib/serialize/serialize.h>
#include <snnlib/protos/model.pb.h>

class Neuron;

class LearningRule : public Serializable<Protos::BlankModel> {
public:
	LearningRule() : Serializable(EBlankModel) {
		blank = false;
	}
	virtual void init(const ConstObj *_c, Neuron *_n)  {
		
	}
    virtual void calculateWeightsDynamics() = 0;
    bool isBlank() {
    	return blank;
    }
protected:
	bool blank;
	Neuron *n;    
};

class BlankLearningRule: public LearningRule  {
public:
	BlankLearningRule()  {
		LearningRule::blank = true;
	}
	void init(const ConstObj *_c, Neuron *_n) {
        n = _n;
        
	}
	
	void deserialize() {}
    ProtoPack serialize() { return ProtoPack(); } 

    void calculateWeightsDynamics() {}
    void print(std::ostream& str) const { }

};
