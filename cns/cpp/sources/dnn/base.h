#pragma once

#include <snn/core.h>

#include <snn/util/fast_delegate.h>
using namespace fastdelegate;

class SynSpike;

typedef FastDelegate0<> stateDelegate;
typedef FastDelegate1<const double&, double> funDelegate;
typedef FastDelegate1<const SynSpike&> propSynSpikeDelegate;
typedef FastDelegate1<const double&> attachDelegate;
typedef FastDelegate0<const bool&> getBoolDelegate;
typedef FastDelegate0<const double&> getDoubleDelegate;


struct SynSpike {
    double t;
    size_t n_id;
    size_t syn_id;
};
