#pragma once


#include <snnlib/base.h>
#include <snnlib/config/constants.h>


class Synapse : public Printable {
private:
    Synapse() {}
    friend class Factory;
public:
    Synapse(const ConstObj *_c, size_t _id_pre, double _w, double _dendrite_delay) {
        init(_c, _w, _id_pre, _dendrite_delay);
    }
    void init(const ConstObj *_c, size_t _id_pre, double _w, double _dendrite_delay) {
        CAST_TYPE(SynapseC, _c)
        c = cast;
        id_pre = _id_pre;
        w = _w;
        x = 0.0;
        dendrite_delay = _dendrite_delay;
    }

    size_t id_pre;

    double x;
    double w;
    double dendrite_delay;

    uchar fired;

    void propagateSpike() {
        x += c->amp;
    }
    void print(std::ostream& str) const {
        str << "Synapse(id_pre: " << id_pre << ", x:" << x << ", w: " << w << ", dendrite_delay: " << dendrite_delay << ")";
    }

    const SynapseC *c;
};
