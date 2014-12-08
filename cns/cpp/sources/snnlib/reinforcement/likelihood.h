#pragma once


#include "reward_modulation.h"

#include <snnlib/learning/srm_methods.h>

class Likelihood : public RewardModulation {
protected:
    Likelihood() {
        Serializable::init(ELikelihood);
    }
    friend class Factory;

public:
    Likelihood(const LikelihoodC *_c, Neuron *_n, RuntimeGlobals *_glob_c) {
        init(_c, _n, _glob_c);
    }
    void init(const ConstObj *_c, Neuron *_n, RuntimeGlobals *_glob_c) {
        c = castType<LikelihoodC>(_c);
        n = _n;
        glob_c = _glob_c;
        Serializable::init(ELikelihood);
    }
    void modulateReward() {
        if(c->input_target) {
            glob_c->propagateReward(n->id, SRMMethods::LLH_given_Y(n, glob_c->inputNeuronsFiring(n->id)));
        } else {
            glob_c->propagateReward(n->id, SRMMethods::LLH(n));
        }
    }

    void deserialize() {}
    ProtoPack serialize() { return ProtoPack(); }
    void print(std::ostream& str) const { }


    const LikelihoodC *c;
    RuntimeGlobals *glob_c;
    Neuron *n;
};
