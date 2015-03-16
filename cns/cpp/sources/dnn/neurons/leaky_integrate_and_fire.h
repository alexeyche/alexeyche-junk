#pragma once


#include "spike_neuron.h"
#include <dnn/protos/generated.pb.h>
#include <dnn/io/serialize.h>

namespace dnn {


/*@GENERATE_PROTO@*/
struct LeakyIntegrateAndFireC : public Serializable<Protos::LeakyIntegrateAndFireC> {
    LeakyIntegrateAndFireC() : R(1.0), C(50.0) {}

    void serial_process() {
        begin() << "R: " << R << ", " 
                << "C: " << C << ", "
                << "leak: " << leak << Self::end;
    }

    double R;
    double C;
    double leak;
};


/*@GENERATE_PROTO@*/
struct LeakyIntegrateAndFireState : public Serializable<Protos::LeakyIntegrateAndFireState>  {
    LeakyIntegrateAndFireState() : p(0.0), u(0.0), fired(false) {}

    void serial_process() {
        begin() << "p: "       << p << ", " \
                << "u: "       << u << ", " \
                << "fired: "   << fired << Self::end;
    }
    bool fired;
    double p;
    double u;
};


class LeakyIntegrateAndFire : public SpikeNeuron<LeakyIntegrateAndFireC, LeakyIntegrateAndFireState> {
public:
    const string name() const {
        return "LeakyIntegrateAndFire";
    }

    void reset() {
        s.p = 0.0;
        s.u = 0.0;
    }

    void propagateSynapseSpike(const SynSpike &sp) {
        syns[ sp.syn_id ].ifc().propagateSpike();
    }

    void calculateDynamics(const Time& t) {
        double syns_pot = 0.0;
        for(auto &s: syns) {
            syns_pot += s.ifc().getMembranePotential();
        }

        s.u += t.dt * ( -(s.u - c.leak)/c.R + input.ifc().getValue(t) + syns_pot) / c.C; 
        
        if(getUnif() < act_f.ifc().prob(s.u)) {
            s.fired = true;
        }

        stat.add("u", s.u);
    }

    bool pullFiring() {
        bool acc = s.fired;
        s.fired = false;
        return acc;
    }
    const double& getFiringProbability() {
        return s.p;
    }

    void provideInterface(SpikeNeuronInterface &i) {
        i.calculateDynamics = MakeDelegate(this, &LeakyIntegrateAndFire::calculateDynamics);
        i.pullFiring = MakeDelegate(this, &LeakyIntegrateAndFire::pullFiring);
        i.getFiringProbability = MakeDelegate(this, &LeakyIntegrateAndFire::getFiringProbability);
        i.propagateSynapseSpike = MakeDelegate(this, &LeakyIntegrateAndFire::propagateSynapseSpike);
    }
};

}
