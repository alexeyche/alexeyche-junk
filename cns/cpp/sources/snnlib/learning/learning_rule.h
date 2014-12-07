#pragma once

#include <snnlib/base.h>
#include <snnlib/serialize/serialize.h>
#include <snnlib/protos/model.pb.h>
#include <snnlib/config/constants.h>
#include <snnlib/neurons/synapse.h>

class Neuron;

class LearningRule : public Serializable<Protos::BlankModel> {
public:
	LearningRule() : Serializable(EBlankModel) {
		blank = false;
		collectStatistics = false;
	}
	virtual void init(const ConstObj *_c, Neuron *_n)  {
		
	}
	virtual void saveStat(SerialPack &p) {};
    virtual void calculateWeightsDynamics() = 0;
    virtual void addSynapse(Synapse *s) {}
    bool isBlank() {
    	return blank;
    }
	virtual void enableCollectStatistics() {};
protected:
	bool blank;
	Neuron *n;    

	bool collectStatistics;
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
