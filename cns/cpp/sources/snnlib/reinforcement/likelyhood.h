#pragma once


#include "reward_modulation.h"

class Likelihood : public RewardModulation {
protected:
    Likelihood() {
        Serializable::init(ELikelihood);
    }
    friend class Factory;

public:
    Likelihood(const LikelihoodC *_c, Neuron *_n) {
        init(_c, _n);
    }
    void init(const ConstObj *_c, Neuron *_n) {
        c = castType<LikelihoodC>(_c);
        n = _n;

        Serializable::init(ELikelihood);
    }


    void deserialize() {}
    ProtoPack serialize() { return ProtoPack(); }
    void print(std::ostream& str) const { }


    const LikelihoodC *c;
    Neuron *n;
};
