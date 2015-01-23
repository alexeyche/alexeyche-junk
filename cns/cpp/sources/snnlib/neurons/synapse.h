#pragma once


#include <snnlib/base.h>
#include <snnlib/config/constants.h>
#include <snnlib/protos/model.pb.h>
#include <snnlib/serialize/serialize.h>
#include <snnlib/sim/runtime_globals.h>


struct SynapseRuntime {
    stateDelegate calculateDynamics;
    stateDelegate propagateSpike;
};

class Synapse : public Serializable<Protos::Synapse> {
protected:
    Synapse() : Serializable(ESynapse) {}
    friend class Factory;
public:
    Synapse(const ConstObj *_c, size_t _id_pre, double _w, double _dendrite_delay) : Serializable(ESynapse) {
        init(_c, _id_pre, _w, _dendrite_delay);
    }
    void init(const ConstObj *_c, size_t _id_pre, double _w, double _dendrite_delay) {
        c = castType<SynapseC>(_c);
        id_pre = _id_pre;
        w = _w;
        x = 0.0;
        dendrite_delay = _dendrite_delay;
        fired = 0;
    }

    virtual void calculateDynamics() {
        x -= x/c->epsp_decay;
        fired = 0;
    }
    virtual void reset() {
        x = 0;
    }
    virtual void propagateSpike() {
        x += c->amp;
        fired = 1;
    }

    void provideRuntime(SynapseRuntime &srt) {
        srt.calculateDynamics = MakeDelegate(this, &Synapse::calculateDynamics);
        srt.propagateSpike = MakeDelegate(this, &Synapse::propagateSpike);
    }

    size_t id_pre;

    double x;
    double w;
    double dendrite_delay;

    double fired;

    void print(std::ostream& str) const {
        str << "Synapse(id_pre: " << id_pre << ", x:" << x << ", w: " << w << ", dendrite_delay: " << dendrite_delay << ")";
    }
    void deserialize();
    ProtoPack serialize();

    const SynapseC *c;
};
