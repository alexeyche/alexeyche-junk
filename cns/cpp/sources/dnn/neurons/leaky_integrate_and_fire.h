#pragma once


#include "spike_neuron.h"
#include <dnn/protos/generated.pb.h>
#include <dnn/io/serialize.h>

namespace dnn {


/*@GENERATE_PROTO@*/
struct LeakyIntegrateAndFireC : public Serializable<Protos::LeakyIntegrateAndFireC> {
    LeakyIntegrateAndFireC() 
    : 
      tau_m(5.0)
    , rest_pot(0.0) 
    , tau_ref(2.0)
    , noise(0.0)
    {}

    void serial_process() {
        begin() << "tau_m: " << tau_m << ", "
                << "rest_pot: " << rest_pot << ", "
                << "tau_ref: " << tau_ref << ", "
                << "noise: " << noise << Self::end;
    }

    double tau_m;
    double rest_pot;
    double tau_ref;
    double noise;
};


/*@GENERATE_PROTO@*/
struct LeakyIntegrateAndFireState : public Serializable<Protos::LeakyIntegrateAndFireState>  {
    LeakyIntegrateAndFireState() 
    : p(0.0)
    , u(0.0)    
    , ref_time(0.0)
    {}

    void serial_process() {
        begin() << "p: "        << p << ", " 
                << "u: "        << u << ", " 
                << "ref_time: " << ref_time << Self::end;
    }    
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
    }

    void calculateDynamics(const Time& t, const double &Iinput, const double &Isyn) {
        // if((_id == 101)&&(t.t>=2500)) {
        //     cout << "Neuron: (Iinput: " << Iinput << ", " << "Isyn: " << Isyn << ", du: " << t.dt * ( - s.u  + c.noise*getNorm() + Iinput + Isyn) / c.tau_m << ")\n";
        // }
        if(s.ref_time < 0.001) {
            s.u += t.dt * ( - s.u  + c.noise*getNorm() + Iinput + Isyn) / c.tau_m;
            s.p = act_f.ifc().prob(s.u);

            if(s.p > getUnif()) {
                setFired(true);
                s.u = c.rest_pot;
                s.ref_time = c.tau_ref;
            }
        } else {
            s.ref_time -= t.dt;
        }
        stat.add("u", s.u);
        stat.add("p", s.p);
    }
   
    const double& getFiringProbability() {
        return s.p;
    }
};

}
