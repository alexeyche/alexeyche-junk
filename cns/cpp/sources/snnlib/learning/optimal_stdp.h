#pragma once

#include "learning_rule.h"

#include <snnlib/neurons/neuron.h>

class Factory;

class OptimalStdp : public LearningRule  {
protected:
    OptimalStdp() {
        Serializable::init(EOptimalStdp);
    }
    friend class Factory;

public:
    OptimalStdp(const OptimalStdpC *_c, Neuron *_n) {
    	init(_c, _n);
    }
    void init(const ConstObj *_c, Neuron *_n) {
        c = castType<OptimalStdpC>(_c);
        n = _n;
        p_acc = 0.0;

        Serializable::init(EOptimalStdp);
    }

    void calculateWeightsDynamics()  {

    }
    void deserialize() {
        Protos::OptimalStdp *mess = getSerializedMessage<Protos::OptimalStdp>();
        p_acc = mess->p_acc();
    }
    ProtoPack serialize() { 
        Protos::OptimalStdp *mess = getNewMessage<Protos::OptimalStdp>();
        mess->set_p_acc(p_acc);
        return ProtoPack({mess}); 
    }
    ProtoPack getNew() { 
        return ProtoPack({ getNewMessage<Protos::OptimalStdp>() }); 
    }
    
    void print(std::ostream& str) const { }

    double p_acc;

    const OptimalStdpC *c;
    Neuron *n;
};

