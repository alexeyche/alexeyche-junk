#pragma once


#include "learning_rule.h"
#include <dnn/protos/generated.pb.h>
#include <dnn/io/serialize.h>
#include <dnn/util/fastapprox/fastexp.h>

namespace dnn {


/*@GENERATE_PROTO@*/
struct StdpC : public Serializable<Protos::StdpC> {
    StdpC() 
    : tau_plus(30.0)
    , tau_minus(50.0)
    , a_plus(1.0)
    , a_minus(1.5)
    , learning_rate(1.0)
    , w_max(5.0)
    , w_min(0.0)
    {}

    void serial_process() {
        begin() << "tau_plus: " << tau_plus << ", " 
                << "tau_minus: " << tau_minus << ", " 
                << "a_plus: " << a_plus << ", " 
                << "a_minus: " << a_minus << ", " 
                << "learning_rate: " << learning_rate << ", " 
                << "w_max: " << w_max << ", "
                << "w_min: " << w_min <<  Self::end;
    }

    double tau_plus;
    double tau_minus;
    double a_plus;
    double a_minus;
    double learning_rate;
    double w_max;
    double w_min;
};


/*@GENERATE_PROTO@*/
struct StdpState : public Serializable<Protos::StdpState>  {
    StdpState() 
    : y(0.0)
    {}

    void serial_process() {
        begin() << "y: "        << y << ", " 
                << "x: "        << x << Self::end;
    }

    double y;
    ActVector<double> x;
};


class Stdp : public LearningRule<StdpC, StdpState, SpikeNeuronBase> {
public:
    const string name() const {
        return "Stdp";
    }

    void reset() {
        s.y = 0;
        s.x.resize(n.ref().getSynapses().size());
        for(auto &v: s.x) {
            v = 0;
        }
    }

    void propagateSynapseSpike(const SynSpike &sp) {
        s.x[sp.syn_id] += 1;
    }

    void calculateDynamics(const Time& t) {
        if(n.ref().fired()) {
            s.y += 1;
        }
        auto &syns = n.ref().getSynapses();
        
        auto x_id_it = s.x.ibegin();
        //if((n.ref().id() == 101)&&(t.t>=2500)) cout << "Stdp: ";
        while(x_id_it != s.x.iend()) {            
            if(fabs(s.x[x_id_it]) < 0.0001) {
                s.x.setInactive(x_id_it);
            } else {
                const size_t &syn_id = *x_id_it;
                auto &syn = syns.get(syn_id).ref();
                double dw = c.learning_rate * ( 
                    c.a_plus  * s.x[x_id_it] * n.ref().fired() - \
                    c.a_minus * s.y * syn.fired() 
                );
                // if(syns.get(syn_id).ifc().getMembranePotential()<0) {
                //     dw = -dw;
                // }
                double new_weight = syn.weight() + dw;
                if( (new_weight<c.w_max)&&(new_weight>=c.w_min) ) {
                    syn.mutWeight() = new_weight;    
                }

                //if((n.ref().id() == 101)&&(t.t>=2500)) cout << "(id_pre: " << syn.idPre() << ", dw: " << dw << ", s.y: " << s.y << ", s.x: " << s.x[x_id_it] << "), ";
                
                s.x[x_id_it] += - s.x[x_id_it]/c.tau_plus;
                ++x_id_it;
            }
        }
        //if((n.ref().id() == 101)&&(t.t>=2500)) cout << "\n";
        s.y += - s.y/c.tau_minus;
        
        if(stat.on()) {
            size_t i=0; 
            for(auto &syn: syns) {
                stat.add("x", i, s.x.get(i));
                stat.add("w", i, syn.ref().weight());
                ++i;
            }
            stat.add("y", s.y);
        }
    }
    
};

}
