#pragma once

#include <snnlib/base.h>
#include <snnlib/serialize/serialize.h>
#include <snnlib/protos/model.pb.h>
#include <snnlib/config/constants.h>
#include <snnlib/neurons/synapse.h>

#include <snnlib/weight_normalizations/weight_normalization.h>
#include <snnlib/act_funcs/act_func.h>

class Neuron;


struct LearningRuleRuntime {
    propSynSpikeDelegate propagateSynSpike;
    stateDelegate calculateWeightsDynamics;
};

class LearningRule : public Serializable<Protos::BlankModel> {
public:
	LearningRule() : Serializable(EBlankModel) {
		collectStatistics = false;
	}
	virtual void init(const ConstObj *_c, Neuron *_n, ActFunc *_act_f, WeightNormalization *_wnorm) = 0;
	virtual void saveStat(SerialPack &p) {};
    void calculateWeightsDynamics() {};
    void propagateSynSpike(const SynSpike *sp) {}
    virtual void addSynapse(Synapse *s) {}
    virtual void reset() {}
	virtual void enableCollectStatistics() {};
    virtual void provideRuntime(LearningRuleRuntime &rt) = 0;


    static void calculateWeightsDynamicsDefault() {}
    static void propagateSynSpikeDefault(const SynSpike *sp) {}
    static void provideDefaultRuntime(LearningRuleRuntime &rt) {
        rt.propagateSynSpike = &LearningRule::propagateSynSpikeDefault;
        rt.calculateWeightsDynamics = &LearningRule::calculateWeightsDynamicsDefault;
    }

    WeightNormalizationRuntime wnorm_rt;
    ActFuncRuntime act_rt;
protected:
	Neuron *n;
    WeightNormalization *wnorm;

	bool collectStatistics;
};

