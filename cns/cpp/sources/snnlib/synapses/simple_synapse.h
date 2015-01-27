#pragma once


#include "synapse.h"

class SimpleSynapse : public Synapse {
protected:
    SimpleSynapse()  {}
    friend class Factory;
public:
    SimpleSynapse(const ConstObj *_c, size_t _id_pre, double _w, double _dendrite_delay)  {
        init(_c, _id_pre, _w, _dendrite_delay);
    }
    void init(const ConstObj *_c, size_t _id_pre, double _w, double _dendrite_delay) {
    	Synapse::init(_c, _id_pre, _w, _dendrite_delay);
        Serializable::init(ESimpleSynapse);
    	c = castType<SimpleSynapseC>(_c);
    }
    void calculateDynamics() {
        x -= x/c->epsp_decay;
        fired = 0;
    }
    void reset() {
        x = 0;
    }
    void propagateSpike() {
        x += c->amp;
        fired = 1;
    }
    double getCurrent() {
        return x;
    }
    
	void provideRuntime(SynapseRuntime &srt) {
        srt.calculateDynamics = MakeDelegate(this, &SimpleSynapse::calculateDynamics);
        srt.propagateSpike = MakeDelegate(this, &SimpleSynapse::propagateSpike);
    }


    void print(std::ostream& str) const {
        str << "SimpleSynapse(id_pre: " << id_pre << ", x:" << x << ", w: " << w << ", dendrite_delay: " << dendrite_delay << ")";
    }

    const SimpleSynapseC *c;
};    