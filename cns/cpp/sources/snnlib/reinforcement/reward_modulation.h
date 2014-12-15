#pragma once


#include <snnlib/serialize/serialize.h>
#include <snnlib/protos/model.pb.h>

struct RewardModulationRuntime {
    stateDelegate modulateReward;
};


class RewardModulation : public Serializable<Protos::BlankModel> {
public:
    RewardModulation() : Serializable(EBlankModel) {}
	virtual void init(const ConstObj *_c, Neuron *_n, RuntimeGlobals *_glob_c) = 0;
    virtual void modulateReward() = 0;
    virtual void provideRuntime(RewardModulationRuntime &rt) = 0;

    static void modulateRewardDefault() {}
    static void provideDefaultRuntime(RewardModulationRuntime &rt) {
        rt.modulateReward = &RewardModulation::modulateRewardDefault;
    }

};


