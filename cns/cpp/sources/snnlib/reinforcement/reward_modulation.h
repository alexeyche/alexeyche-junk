#pragma once


#include <snnlib/serialize/serialize.h>
#include <snnlib/protos/model.pb.h>


class RewardModulation : public Serializable<Protos::BlankModel> {
public:
    RewardModulation() : Serializable(EBlankModel) {}

};


