#pragma once


#include "spike_neuron.h"
#include <dnn/protos/generated.pb.h>
#include <dnn/io/serialize.h>

namespace dnn {


/*@GENERATE_PROTO@*/
struct LeakyIntegrateAndFireC : public Serializable<Protos::LeakyIntegrateAndFireC> {
    LeakyIntegrateAndFireC() : R(1.0), C(50.0) {}

    void serial_process() {
        begin() << "R: " << R << ", " << "C: " << C << end();
    }

	double R;
	double C;
};


/*@GENERATE_PROTO@*/
struct LeakyIntegrateAndFireState : public Serializable<Protos::LeakyIntegrateAndFireState>  {
    LeakyIntegrateAndFireState() : p(0.0), u(0.0), fired(false) {}

    void serial_process() {
        begin() << "p: "       << p << ", " \
                << "u: "       << u << ", " \
                << "fired: "   << fired << end();
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
        syns[ sp.syn_id ].getInterface().propagateSpike();
    }

    void calculateDynamics(const Time& t) {

    }

    const bool& fired() {
        return s.fired;
    }
    const double& getFiringProbability() {
        return s.p;
    }

    void provideInterface(SpikeNeuronInterface &i) {
        i.calculateDynamics = MakeDelegate(this, &LeakyIntegrateAndFire::calculateDynamics);
        i.fired = MakeDelegate(this, &LeakyIntegrateAndFire::fired);
        i.getFiringProbability = MakeDelegate(this, &LeakyIntegrateAndFire::getFiringProbability);
        i.propagateSynapseSpike = MakeDelegate(this, &LeakyIntegrateAndFire::propagateSynapseSpike);
    }
};

}
