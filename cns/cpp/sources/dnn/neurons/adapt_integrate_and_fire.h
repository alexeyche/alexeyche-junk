#pragma once


#include "spike_neuron.h"
#include <dnn/protos/generated.pb.h>
#include <dnn/io/serialize.h>

namespace dnn {


/*@GENERATE_PROTO@*/
struct AdaptIntegrateAndFireC : public Serializable<Protos::AdaptIntegrateAndFireC> {
    AdaptIntegrateAndFireC() 
    : gL(0.1)
    , C(1.0)
    , leak(-65.0)
    , rest_pot(-70.0) 
    , tau_ref(2.0)
    , noise(1.5)
    , tau_adapt(80.0)
    , kd(30.0)
    , vK(-80.0)
    , adapt_amp(0.2)
    , gKCa(5.0)
    {}

    void serial_process() {
        begin() << "gL: " << gL << ", " 
                << "C: " << C << ", "
                << "leak: " << leak << ", "
                << "rest_pot: " << rest_pot << ", "
                << "tau_ref: " << tau_ref << ", "
                << "noise: " << noise << ", "
                << "tau_adapt: " << tau_adapt << ", "
                << "kd: " << kd << ", "
                << "vK: " << vK << ", "
                << "adapt_amp: " << adapt_amp << ", "
                << "gKCa: " << gKCa << Self::end;
    }

    double gL;
    double C;
    double leak;
    double rest_pot;
    double tau_ref;
    double noise;
    double tau_adapt;
    double kd;
    double vK;
    double adapt_amp;
    double gKCa;
};


/*@GENERATE_PROTO@*/
struct AdaptIntegrateAndFireState : public Serializable<Protos::AdaptIntegrateAndFireState>  {
    AdaptIntegrateAndFireState() 
    : p(0.0)
    , u(0.0)
    , fired(false)
    , ref_time(0.0)
    , Ca(0.0)
    {}

    void serial_process() {
        begin() << "p: "        << p << ", " 
                << "u: "        << u << ", " 
                << "ref_time: " << ref_time << ", " 
                << "fired: "    << fired << ", "
                << "Ca: "       << Ca << Self::end;
    }
    bool fired;
    double p;
    double u;
    double ref_time;
    double Ca;
};


class AdaptIntegrateAndFire : public SpikeNeuron<AdaptIntegrateAndFireC, AdaptIntegrateAndFireState> {
public:
    const string name() const {
        return "AdaptIntegrateAndFire";
    }

    void reset() {
        s.p = 0.0;
        s.u = c.rest_pot;
        s.ref_time = 0.0;
        s.fired = false;
        s.Ca = 0.0;
    }

    void propagateSynapseSpike(const SynSpike &sp) {
        syns[ sp.syn_id ].ifc().propagateSpike();
        lrule.ifc().propagateSynapseSpike(sp);
    }

    void calculateDynamics(const Time& t) {
        readInputSpikes(t);
        
        const double& input_current = input.ifc().getValue(t);
        
        
        if(s.ref_time < 0.001) {
            double syns_pot = 0.0;
            for(auto &s: syns) {
                double x = s.ifc().getMembranePotential();
                syns_pot += x;
            }
            double Ia = c.gKCa * (s.Ca/(s.Ca + c.kd)) * (s.u - c.vK);
            stat.add("Ia", Ia);
            
            s.u += t.dt * ( 
                - c.gL * (s.u - c.leak) 
                + c.noise * getNorm() 
                + input_current 
                + syns_pot 
                - Ia
                ) / c.C;
            s.p = act_f.ifc().prob(s.u);
            
            
            if(getUnif() < s.p) {
                s.fired = true;
                s.u = c.rest_pot;
                s.ref_time = c.tau_ref;
                s.Ca += c.adapt_amp;
            }
        } else {
            s.ref_time -= t.dt;
        }

        s.Ca += t.dt * ( - s.Ca/c.tau_adapt ); 
        for(auto &s: syns) {
            s.ifc().calculateDynamics(t);
        }
        stat.add("u", s.u);
        stat.add("Ca", s.Ca);
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
        i.calculateDynamics = MakeDelegate(this, &AdaptIntegrateAndFire::calculateDynamics);
        i.pullFiring = MakeDelegate(this, &AdaptIntegrateAndFire::pullFiring);
        i.getFiringProbability = MakeDelegate(this, &AdaptIntegrateAndFire::getFiringProbability);
        i.propagateSynapseSpike = MakeDelegate(this, &AdaptIntegrateAndFire::propagateSynapseSpike);
    }
};

}
