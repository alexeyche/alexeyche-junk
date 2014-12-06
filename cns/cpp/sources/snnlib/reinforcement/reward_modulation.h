#pragma once


#include <snnlib/serialize/serialize.h>
#include <snnlib/protos/model.pb.h>


class RewardModulation : public Serializable<Protos::BlankModel> {
public:
    RewardModulation() : Serializable(EBlankModel) {}
	virtual void init(const ConstObj *_c, Neuron *_n, RuntimeGlobals *_glob_c) = 0;
    virtual void modulateReward() = 0;
};


