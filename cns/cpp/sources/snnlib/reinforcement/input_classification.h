#pragma once


#include "reward_modulation.h"

#include <snnlib/learning/srm_methods.h>

class InputClassification : public RewardModulation {
protected:
    InputClassification() {
        Serializable::init(EInputClassification);
    }
    friend class Factory;

public:
    InputClassification(const InputClassificationC *_c, Neuron *_n, RuntimeGlobals *_glob_c) {
        init(_c, _n, _glob_c);
    }
    void init(const ConstObj *_c, Neuron *_n, RuntimeGlobals *_glob_c) {
        c = castType<InputClassificationC>(_c);
        n = _n;
        glob_c = _glob_c;
        Serializable::init(EInputClassification);
    }
    void modulateReward() {
        if(n->fired) {
            if(n->local_id == *glob_c->current_class_id) {                
                glob_c->propagateReward(n->id, c->ltp);
                //cout << c->ltp << "\n";
            } else {
                glob_c->propagateReward(n->id, c->ltd);
                //cout << c->ltd << "\n";
            }
        }
    }

    void deserialize() {}
    ProtoPack serialize() { return ProtoPack(); }
    void print(std::ostream& str) const { }


    const InputClassificationC *c;
    RuntimeGlobals *glob_c;
    Neuron *n;
};
