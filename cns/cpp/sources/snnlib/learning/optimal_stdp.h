#pragma once


#include <snnlib/neurons/neuron.h>
#include <snnlib/serialize/serialize.h>
#include <snnlib/weight_normalizations/weight_normalization.h>

#include "learning_rule.h"
#include "srm_methods.h"


class Factory;
class OptimalStdp;

class OptimalStdpStat : public Serializable<Protos::OptimalStdpStat> {
protected:
    OptimalStdpStat() : Serializable<Protos::OptimalStdpStat>(EOptimalStdpStat) { }
    friend class Factory;
public:
    OptimalStdpStat(OptimalStdp *m);

    void collect(OptimalStdp *m);

    ProtoPack serialize() {
        Protos::OptimalStdpStat *stat = getNewMessage();
        for(auto it=C.begin(); it != C.end(); ++it) {
            Protos::OptimalStdpStat::CStat* c_stat = stat->add_c_vals();
            for(auto it_val=it->begin(); it_val != it->end(); ++it_val) {
                c_stat->add_c(*it_val);
            }
        }
        for(auto it=B.begin(); it != B.end(); ++it) {
            stat->add_b(*it);
        }
        for(auto it=p_acc.begin(); it != p_acc.end(); ++it) {
            stat->add_p_acc(*it);
        }
        return ProtoPack({stat});
    }

    virtual void deserialize() {
        Protos::OptimalStdpStat * m = getSerializedMessage();
        for(size_t i=0; i<m->c_vals_size(); i++) {
            Protos::OptimalStdpStat::CStat c_m = m->c_vals(i);

            vector<double> x_v;
            for(size_t j=0; j<c_m.c_size(); j++) {
                x_v.push_back(c_m.c(j));
            }
            C.push_back(x_v);
        }
        for(size_t i=0; i<m->b_size(); i++) {
            B.push_back(m->b(i));
        }
        for(size_t i=0; i<m->p_acc_size(); i++) {
            p_acc.push_back(m->p_acc(i));
        }
    }

    void print(std::ostream& str) const {
    }
    vector<double> p_acc;
    vector<double> B;
    vector<vector<double>> C;
};



class OptimalStdp : public LearningRule  {
protected:
    OptimalStdp() : LearningRule() {
        Serializable::init(EOptimalStdp);
    }
    friend class Factory;

public:
    OptimalStdp(const ConstObj *_c, Neuron *_n, ActFunc *_act_f, WeightNormalization *_wnorm) : LearningRule() {
    	init(_c, _n, _act_f, _wnorm);
    }

    void init(const ConstObj *_c, Neuron *_n, ActFunc *_act_f, WeightNormalization *_wnorm) {
        c = castType<OptimalStdpC>(_c);
        n = _n;
        p_acc = 0.0;
        B = 0.0;
        C.resize(n->syns.size());
        fill(C.begin(), C.end(), 0.0);
        stat = nullptr;
        wnorm = _wnorm;

        _act_f->provideRuntime(act_rt);
        if(_wnorm) {
            _wnorm->provideRuntime(wnorm_rt, WeightNormalization::Derivative);
        } else {
            WeightNormalization::provideDefaultRuntime(wnorm_rt);
        }

        Serializable::init(EOptimalStdp);
    }
    void addSynapse(Synapse *s) {
        C.push_back(0.0);
        if(collectStatistics) {
            stat->C.push_back(vector<double>());
        }
    }

    void enableCollectStatistics() {
        collectStatistics = true;
        stat = Factory::inst().registerObj<OptimalStdpStat>(new OptimalStdpStat(this));
    }

    inline double B_calc() {
        if( fabs(p_acc - 0.0) < 0.00001 ) return(0);
     //printf("p_acc %f, 1part: %f, 2part: %f\n", p_acc, ( n->fired * log( n->p/p_acc) - (n->p - p_acc)), c->target_rate_factor * ( n->fired * log( p_acc/c->__target_rate) - (p_acc - c->__target_rate) ));
     //printf("yspike:%f p: %f pmean:%f log:%f\n", n->fired, n->p, p_acc, log( n->p/p_acc));
        return                         (( n->fired * log(n->p   /         p_acc) - (n->p    -          p_acc)) -  \
                c->target_rate_factor * ( n->fired * log(p_acc/c->__target_rate) - (p_acc - c->__target_rate)) );

    }
    void propagateSynSpike(const SynSpike *sp) {
        if((n->glob_c->getSimTime() >= c->mean_p_dur)&&(fabs(C[sp->syn_id]) < SYN_ACT_TOL)) {
            active_synapses.push_back(sp->syn_id);
        }
    }
    void calculateWeightsDynamics()  {
        wnorm_rt.preModifyMeasure();

        if(n->p<(1.0/1000.0)) n->p = 1.0/1000;
        if(n->glob_c->getSimTime() >= c->mean_p_dur) {
            B = B_calc();
            auto it=active_synapses.begin();
            while(it != active_synapses.end()) {
                Synapse *syn = n->syns[*it];

                C[*it] += - C[*it]/c->tau_c + SRMMethods::dLLH_dw(n, syn);
                double dw = c->learning_rate * ( C[*it]*B - c->weight_decay * syn->fired * syn->w);
                //cout << c->weight_decay << "*" << syn->fired << "*" << syn->w << " == " << c->weight_decay * syn->fired * syn->w << "\n";
                wnorm_rt.modifyWeightDerivative(dw, *it);
                syn->w += dw;

                if(fabs(C[*it]) < SYN_ACT_TOL) {
                    it = active_synapses.erase(it);
                } else {
                    it++;
                }

                if(std::isnan(dw)) {
                    cout << "Found nan dw:\n";
                    double fired = n->fired;
                    cout << n->act_rt.probDeriv(n->y) << "/(" << n->p  << "/" << n->M << ") * (" << fired << "-" << n->p << ") * " << syn->x << " == ";
                    cout << SRMMethods::dLLH_dw(n, syn) << "\n";
                    cout << *n;
                    terminate();
                }
            }
            if(collectStatistics) {
                stat->collect(this);
            }
        }
        p_acc += (-p_acc+n->fired)/c->mean_p_dur;

    }
    void provideRuntime(LearningRuleRuntime &rt) {
        rt.calculateWeightsDynamics = MakeDelegate(this, &OptimalStdp::calculateWeightsDynamics);
        rt.propagateSynSpike = MakeDelegate(this, &OptimalStdp::propagateSynSpike);
    }


    void deserialize() {
        Protos::OptimalStdp *mess = getSerializedMessage<Protos::OptimalStdp>();
        p_acc = mess->p_acc();
    }
    ProtoPack serialize() {
        Protos::OptimalStdp *mess = getNewMessage<Protos::OptimalStdp>();
        mess->set_p_acc(p_acc);
        return ProtoPack({mess});
    }
    ProtoPack getNew() {
        return ProtoPack({ getNewMessage<Protos::OptimalStdp>() });
    }
    void saveStat(SerialPack &p) {
        p.push_back(stat);
    }

    void print(std::ostream& str) const { }

    double p_acc;
    vector<double> C;
    double B;
    list<size_t> active_synapses;

    const OptimalStdpC *c;
    OptimalStdpStat *stat;
    Neuron *n;

};

