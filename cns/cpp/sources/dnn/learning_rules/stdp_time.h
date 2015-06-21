#pragma once


#include "learning_rule.h"
#include <dnn/protos/generated.pb.h>
#include <dnn/io/serialize.h>
#include <dnn/util/fastapprox/fastexp.h>

namespace dnn {


/*@GENERATE_PROTO@*/
struct StdpTimeC : public Serializable<Protos::StdpTimeC> {
    StdpTimeC() 
    : tau_plus(11.0)
    , tau_minus(10.0)
    , a_plus(0.0016)
    , a_minus(0.0055)
    , learning_rate(1.0)
    , w_max(1.0)
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
struct StdpTimeState : public Serializable<Protos::StdpTimeState>  {
    StdpTimeState() 
    : y(-1.0)
    {}

    void serial_process() {
        begin() << "y: "        << y << ", " 
                << "x: "        << x << Self::end;
    }

    double y;
    ActVector<double> x;
};


class StdpTime : public LearningRule<StdpTimeC, StdpTimeState, SpikeNeuronBase> {
public:
    const string name() const {
        return "StdpTime";
    }

    void reset() {
        s.y = -1.0;
        s.x.resize(n.ref().getSynapses().size());
        for(auto &v: s.x) {
            v = -1.0;
        }
    }

    void propagateSynapseSpike(const SynSpike &sp) {
        s.x[sp.syn_id] = sp.t;
    }
    static constexpr double WINDOW_LEN = 100.0;

    void calculateDynamics(const Time& t) {
        if(n.ref().fired()) {
            s.y = t.t;
        }
        auto &syns = n.ref().getSynapses();
        
        auto x_id_it = s.x.ibegin();
        //if((n.ref().id() == 101)&&(t.t>=2500)) cout << "StdpTime: ";
        while(x_id_it != s.x.iend()) {            
            double time_diff = s.y - s.x[x_id_it];
            if(fabs(time_diff) >= WINDOW_LEN) {
                s.x.setInactive(x_id_it);
            } else {
                const size_t &syn_id = *x_id_it;
                auto &syn = syns.get(syn_id).ref();
                if( (!(n.ref().fired()||syn.fired()) )||(s.x[x_id_it]<0.0)||(s.y<0.0)) {
                    ++x_id_it;
                    continue;
                }
                double dw;
                if(time_diff>=0.0) {
                    dw = c.learning_rate * exp(-syn.weight()) * c.a_plus * exp(-time_diff/c.tau_plus); // LTP 
                } else {
                    dw = - c.learning_rate * syn.weight() * c.a_minus * exp(time_diff/c.tau_minus);  // LTD
                }
                
                // if(syns.get(syn_id).ifc().getMembranePotential()<0) {
                //     dw = -dw;
                // }
                double new_weight = syn.weight() + dw;
                if( (new_weight<c.w_max)&&(new_weight>=c.w_min) ) {
                    syn.mutWeight() = new_weight;    
                }

                //if((n.ref().id() == 101)&&(t.t>=2500)) cout << "(id_pre: " << syn.idPre() << ", dw: " << dw << ", s.y: " << s.y << ", s.x: " << s.x[x_id_it] << "), ";
                ++x_id_it;
            }
        }
        //if((n.ref().id() == 101)&&(t.t>=2500)) cout << "\n";
        
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
