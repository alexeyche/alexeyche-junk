#pragma once


#include <snnlib/neurons/neuron.h>
#include <snnlib/serialize/serialize.h>
#include <snnlib/weight_normalizations/weight_normalization.h>

#include "learning_rule.h"
#include "srm_methods.h"


class Factory;
class Stdp;

class StdpStat : public Serializable<Protos::StdpStat> {
protected:
    StdpStat() : Serializable<Protos::StdpStat>(EStdpStat) { }
    friend class Factory;
public:
    StdpStat(Stdp *m);

    void collect(Stdp *m);

    ProtoPack serialize() {
        Protos::StdpStat *stat = getNewMessage();
        for(auto it=x_trace.begin(); it != x_trace.end(); ++it) {
            Protos::StdpStat::XTrace* x_trace_stat = stat->add_x_trace_vals();
            for(auto it_val=it->begin(); it_val != it->end(); ++it_val) {
                x_trace_stat->add_x(*it_val);
            }
        }
        for(auto it=y_trace.begin(); it != y_trace.end(); ++it) {
            stat->add_y_trace(*it);
        }
        return ProtoPack({stat});
    }

    virtual void deserialize() {
        Protos::StdpStat * m = getSerializedMessage();
        for(size_t i=0; i<m->x_trace_vals_size(); i++) {
            Protos::StdpStat::XTrace x_m = m->x_trace_vals(i);

            vector<double> x_v;
            for(size_t j=0; j<x_m.x_size(); j++) {
                x_v.push_back(x_m.x(j));
            }
            x_trace.push_back(x_v);
        }
        for(size_t i=0; i<m->y_trace_size(); i++) {
            y_trace.push_back(m->y_trace(i));
        }
    }

    void print(std::ostream& str) const {
    }
    vector<double> y_trace;
    vector<vector<double>> x_trace;
};



class Stdp : public LearningRule  {
protected:
    Stdp() : LearningRule() {
        Serializable::init(EStdp);
    }
    friend class Factory;
public:
    Stdp(const ConstObj *_c, Neuron *_n, ActFunc *_act_f, WeightNormalization *_wnorm) : LearningRule() {
    	init(_c, _n, _act_f, _wnorm);
    }

    void init(const ConstObj *_c, Neuron *_n, ActFunc *_act_f, WeightNormalization *_wnorm) {
        c = castType<StdpC>(_c);
        n = _n;
        wnorm = _wnorm;
        y_trace = 0.0;
        x_trace.resize(n->syns.size());
        fill(x_trace.begin(), x_trace.end(), 0.0);

        stat = nullptr;
        _act_f->provideRuntime(act_rt);
        WeightNormalization::provideDefaultRuntime(wnorm_rt);
        if(_wnorm) {
            _wnorm->provideRuntime(wnorm_rt, WeightNormalization::LtpLtd);
        }

        Serializable::init(EStdp);
    }
    void addSynapse(Synapse *s) {
        x_trace.push_back(0.0);
        if(collectStatistics) {
            stat->x_trace.push_back(vector<double>());
        }
    }

    void enableCollectStatistics() {
        collectStatistics = true;
        stat = Factory::inst().registerObj<StdpStat>(new StdpStat(this));
    }

    void propagateSynSpike(const SynSpike *sp) {
        if(fabs(x_trace[sp->syn_id]) < SYN_ACT_TOL) {
            active_synapses.push_back(sp->syn_id);
        }
        x_trace[sp->syn_id] += 1;
    }

    void calculateWeightsDynamics()  {
        wnorm_rt.preModifyMeasure();

        if(n->fired) {
            y_trace += 1;
        }

        auto it=active_synapses.begin();
        while(it != active_synapses.end()) {
            Synapse *syn = n->syns[*it];
            //cout << n->id << ":" << *it << "\n";
            //cout <<  c->a_plus  << "*" << wnorm_rt.ltpMod(syn->w) << "*" << x_trace[*it] << "*"  << n->fired  << " == " << c->a_plus  * wnorm_rt.ltpMod(syn->w) * x_trace[*it] * n->fired << "\n";
            double dw = c->learning_rate * ( c->a_plus  * wnorm_rt.ltpMod(syn->w) * x_trace[*it] * n->fired -  \
                                             c->a_minus * wnorm_rt.ltdMod(syn->w) * y_trace      * syn->fired );

            wnorm_rt.modifyWeightDerivative(dw, *it);
            syn->w += dw;

            x_trace[*it] += - x_trace[*it]/c->tau_plus;

            if(fabs(x_trace[*it]) < SYN_ACT_TOL) {
                it = active_synapses.erase(it);
            } else {
                it++;
            }
        }

        y_trace += - y_trace/c->tau_minus;

        if(collectStatistics) {
            stat->collect(this);
        }
    }

    void provideRuntime(LearningRuleRuntime &rt) {
        rt.calculateWeightsDynamics = MakeDelegate(this, &Stdp::calculateWeightsDynamics);
        rt.propagateSynSpike = MakeDelegate(this, &Stdp::propagateSynSpike);
    }


    void deserialize() {}
    ProtoPack serialize() {
        return ProtoPack();
    }
    ProtoPack getNew() {
        return ProtoPack();
    }
    void saveStat(SerialPack &p) {
        p.push_back(stat);
    }

    void print(std::ostream& str) const { }


    double y_trace;
    vector<double> x_trace;
    list<size_t> active_synapses;

    const StdpC *c;
    StdpStat *stat;
    Neuron *n;
};

