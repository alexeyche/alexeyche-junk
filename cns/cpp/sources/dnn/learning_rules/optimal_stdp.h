#pragma once


#include "learning_rule.h"
#include <dnn/protos/generated.pb.h>
#include <dnn/io/serialize.h>
#include <dnn/util/fastapprox/fastexp.h>
#include <dnn/neurons/srm_neuron.h>

namespace dnn {


/*@GENERATE_PROTO@*/
struct OptimalStdpC : public Serializable<Protos::OptimalStdpC> {
    OptimalStdpC() 
    : tau_c(100.0)
    , tau_mean(10000.0)
    , target_rate(10.0)
    , target_rate_factor(1.0)
    , weight_decay(0.0026)
    , learning_rate(0.01)
    {}

    void serial_process() {
        begin() str <<
            "tau_c: " << tau_c << ", " <<
            "tau_mean: " << tau_mean << ", " <<
            "target_rate: " << target_rate << ", " <<
            "target_rate_factor: " << target_rate_factor << ", " <<
            "learning_rate: " << learning_rate << ", " <<
            "weight_decay: " << weight_decay  <<  Self::end;
        __target_rate = target_rate/1000.0;
    }

    double tau_c;
    double tau_mean;
    double target_rate;
    double __target_rate;
    double target_rate_factor;
    double weight_decay;
    double learning_rate;
};


/*@GENERATE_PROTO@*/
struct OptimalStdpState : public Serializable<Protos::OptimalStdpState>  {
    OptimalStdpState() 
    : B(0.0), p_mean(0.0)
    {}

    void serial_process() {
        begin() << "p_mean: " << p_mean << ", " 
                << "C: " << C << ", " 
                << "B: " << B << Self::end;
    }
    double p_mean;
    ActVector<double> C;
    double B;
};


class OptimalStdp : public LearningRule<OptimalStdpC, OptimalStdpState, SRMNeuron> {
public:
    const string name() const {
        return "OptimalStdp";
    }

    void reset() {
        s.B = 0.0;
        s.C.resize(n->getSynapses().size());
        fill(C.begin(), C.end(), 0.0);
        s.p_mean = 0.0;        
    }

    void propagateSynapseSpike(const SynSpike &sp) {
        s.C[sp.syn_id] += SRMMethods::dLLH_dw(n, n->getSynapses().get(sp.syn_id).ref());
    }
    inline double B_calc() const {
        if( fabs(c.p_mean) < 0.00001 ) return(0);
        return                        (( n->fired() * log(n->getFiringProbability()/c.p_mean) - (n->getFiringProbability() - c.p_mean)) -  \
                c.target_rate_factor * ( n->fired() * log(s.p_mean/c.__target_rate) - (c.p_mean - c.__target_rate)) );

    }
    void calculateDynamics(const Time& t) {
        if(n->fired()) {
            s.y += 1;
        }
        auto &syns = n->getSynapses();
        
        auto x_id_it = s.x.ibegin();
        while(x_id_it != s.x.iend()) {            
            if(fabs(s.x[x_id_it]) < 0.0001) {
                s.x.setInactive(x_id_it);
            } else {
                const size_t &syn_id = *x_id_it;
                auto &syn = syns.get(syn_id).ref();
                double dw = c.learning_rate * ( 
                    c.a_plus  * s.x[x_id_it] * n->fired() - \
                    c.a_minus * s.y * syn.fired() 
                );
                double new_weight = syn.weight() + dw;
                if( (new_weight<c.w_max)&&(new_weight>=c.w_min) ) {
                    syn.mutWeight() = new_weight;    
                }

                
                s.x[x_id_it] += - s.x[x_id_it]/c.tau_plus;
                ++x_id_it;
            }
        }
        //if((n->id() == 101)&&(t.t>=2500)) cout << "\n";
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
