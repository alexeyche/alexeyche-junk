#pragma once


#include <dnn/core.h>
#include <dnn/util/fast_delegate.h>

using namespace fastdelegate;

namespace dnn {


class Object {
public:
    virtual ~Object() {}
};

class Printable: public Object {
protected:
    virtual void print(std::ostream& str) const = 0;
public:
    friend std::ostream& operator<<(std::ostream& str, const Printable &data) {
        data.print(str);
        return str;
    }
};


typedef map<string, string> OptMods;

struct SynSpike;
struct Time;


typedef FastDelegate0<> stateDelegate;
typedef FastDelegate1<const double&, double> funDelegate;
typedef FastDelegate1<const SynSpike&> propSynSpikeDelegate;
typedef FastDelegate1<const double&> attachDelegate;
typedef FastDelegate0<const bool&> getBoolDelegate;
typedef FastDelegate0<bool> getBoolCopyDelegate;
typedef FastDelegate0<const double&> getDoubleDelegate;
typedef FastDelegate0<double> retDoubleDelegate;
typedef FastDelegate1<const Time&> calculateDynamicsDelegate;
typedef FastDelegate1<const size_t&, const double&> getValueAtIndexDelegate;

struct SynSpike {
    double t;
    size_t n_id;
    size_t syn_id;
};

struct Time {
    Time(double _dt) : t(0), dt(_dt) {}
    void operator ++() {
        t+=dt;
    }
    bool operator<(const double &dur) {
        return t < dur;
    }
    double t;
    double dt;
};


}
