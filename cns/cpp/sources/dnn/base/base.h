#pragma once


#include <dnn/core.h>
#include <dnn/util/fast_delegate.h>

using namespace fastdelegate;

namespace dnn {


class Object {
public:
    ~Object() {}
};

struct SynSpike;
struct Time;


typedef FastDelegate0<> stateDelegate;
typedef FastDelegate1<const double&, double> funDelegate;
typedef FastDelegate1<const SynSpike&> propSynSpikeDelegate;
typedef FastDelegate1<const double&> attachDelegate;
typedef FastDelegate0<const bool&> getBoolDelegate;
typedef FastDelegate0<const double&> getDoubleDelegate;
typedef FastDelegate0<double> retDoubleDelegate;
typedef FastDelegate0<const Time&> calculateDynamicsDelegate;

struct SynSpike {
    double t;
    size_t n_id;
    size_t syn_id;
};

struct Time {
    double t;
    double dt;
};


}
