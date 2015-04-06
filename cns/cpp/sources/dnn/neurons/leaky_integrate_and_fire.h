#pragma once


#include "spike_neuron.h"
#include <dnn/protos/generated.pb.h>
#include <dnn/io/serialize.h>

namespace dnn {


/*@GENERATE_PROTO@*/
struct LeakyIntegrateAndFireC : public Serializable<Protos::LeakyIntegrateAndFireC> {
    LeakyIntegrateAndFireC() 
    : gL(0.1)
    , C(1.0)
    , leak(5.0)
    , rest_pot(0.0) 
    , tau_ref(2.0)
    , noise(0.0)
    {}

    void serial_process() {
        begin() << "gL: " << gL << ", " 
                << "C: " << C << ", "
                << "leak: " << leak << ", "
                << "rest_pot: " << rest_pot << ", "
                << "tau_ref: " << tau_ref << ", "
                << "noise: " << noise << Self::end;
    }

    double gL;
    double C;
    double leak;
    double rest_pot;
    double tau_ref;
    double noise;
};


/*@GENERATE_PROTO@*/
struct LeakyIntegrateAndFireState : public Serializable<Protos::LeakyIntegrateAndFireState>  {
    LeakyIntegrateAndFireState() 
    : p(0.0)
    , u(0.0)
    , fired(false)
    , ref_time(0.0)
    {}

    void serial_process() {
        begin() << "p: "        << p << ", " 
                << "u: "        << u << ", " 
                << "ref_time: " << ref_time << ", " 
                << "fired: "    << fired << Self::end;
    }
    bool fired;
    double p;
    double u;
    double ref_time;
};


class LeakyIntegrateAndFire : public SpikeNeuron<LeakyIntegrateAndFireC, LeakyIntegrateAndFireState> {
public:
    const string name() const {
        return "LeakyIntegrateAndFire";
    }

    void reset() {
        s.p = 0.0;
        s.u = c.rest_pot;
        s.ref_time = 0.0;
        s.fired = false;
    }

    void propagateSynapseSpike(const SynSpike &sp) {
        syns[ sp.syn_id ].ifc().propagateSpike();
        lrule.ifc().propagateSynapseSpike(sp);
    }

    void calculateDynamics(const Time& t, const double &Iinput, const double &Isyn) {
        if(s.ref_time < 0.001) {
            s.u += t.dt * ( - c.gL * (s.u - c.leak) + c.noise*getNorm() + Iinput + Isyn) / c.C;
            s.p = act_f.ifc().prob(s.u);
            
            if(getUnif() < s.p) {
                s.fired = true;
                s.u = c.rest_pot;
                s.ref_time = c.tau_ref;
            }
        } else {
            s.ref_time -= t.dt;
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
};

}
