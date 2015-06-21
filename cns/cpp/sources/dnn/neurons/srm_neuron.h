#pragma once

#include <dnn/util/fastapprox/fastexp.h>
#include "spike_neuron.h"
#include <dnn/protos/generated.pb.h>
#include <dnn/io/serialize.h>

namespace dnn {


/*@GENERATE_PROTO@*/
struct SRMNeuronC : public Serializable<Protos::SRMNeuronC> {
    SRMNeuronC() 
    : u_rest(0.0)
    , amp_adapt(1.0) 
    , amp_refr(-100.0)
    , tau_refr(2.0)
    , tau_adapt(50.0)
    {}

    void serial_process() {
        begin()  << "u_rest: " << u_rest << ", " <<
    			    "amp_refr: " << amp_refr << ", " <<
                    "amp_adapt: " << amp_adapt << ", " <<    			
    				"tau_refr: " << tau_refr << ", " <<
        			"tau_adapt: " << tau_adapt  << Self::end;
    }

    double u_rest;
    double amp_refr;
    double amp_adapt;
    double tau_refr;
    double tau_adapt;
};


/*@GENERATE_PROTO@*/
struct SRMNeuronState : public Serializable<Protos::SRMNeuronState>  {
    SRMNeuronState() 
    : p(0.0)
    , u(0.0)    
    , M(1.0)
    , gr(0.0)
    , ga(0.0)
    {}

    void serial_process() {
        begin() << "p: " << p << ", " 
                << "u: " << u << ", " 
                << "gr: " << gr << ", " 
                << "ga: " << ga << ", " 
                << "M: " << M << Self::end;
    }

    double u;  
    double p;
    double M;
    double gr;
    double ga;
};


class SRMNeuron : public SpikeNeuron<SRMNeuronC, SRMNeuronState> {
public:
    const string name() const {
        return "SRMNeuron";
    }

    void reset() {
        s = SRMNeuronState();
    }

    void calculateDynamics(const Time& t, const double &Iinput, const double &Isyn) {
        s.u = c.u_rest + Iinput + Isyn;
        s.M = fastexp(-(s.gr+s.ga));
        s.p = act_f.ifc().prob(s.u) * s.M;
        
        if(s.p > getUnif()) {
            setFired(true);
            s.gr += c.amp_refr;
            s.ga += c.amp_adapt;
        }
        s.gr += - s.gr/c.tau_refr;
        s.ga += - s.ga/c.tau_adapt;
        
        stat.add("u", s.u);
        stat.add("p", s.p);
        stat.add("M", s.M);
        stat.add("ga", s.ga);
    }
   
    const double& getFiringProbability() {
        return s.p;
    }
    const double& getProbabilityModulation() {
        return s.M;
    }
};

}
