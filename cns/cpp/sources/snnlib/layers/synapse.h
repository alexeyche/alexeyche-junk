#pragma once

#include "common.h"

#include <snnlib/base.h>
#include <snnlib/config/constants.h>

typedef priority_queue<SynSpike, vector<SynSpike>, CompareSynSpike> SpikeQueue;

class Synapse : public Printable {
private:
    Synapse() {}
    friend class Factory;
public:
    Synapse(const ConstObj *_c, size_t _id_pre, double _w) {
        init(_c, _w, _id_pre);
    }
    void init(const ConstObj *_c, size_t _id_pre, double _w) {
        CAST_TYPE(SynapseC, _c)
        c = cast;
        id_pre = _id_pre;
        w = _w;
        x = 0.0;
    }

    size_t id_pre;

    double x;
    double w;

    uchar fired;

    SpikeQueue q;
    void propagateSpike() {
        x += c->amp;
    }
    void print(std::ostream& str) const {
        str << "Synapse(id_pre: " << id_pre << ", x:" << x << ", w: " << w << ")";
    }

    const SynapseC *c;
};
