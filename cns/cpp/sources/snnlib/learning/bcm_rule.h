#pragma once


#include <snnlib/neurons/neuron.h>
#include <snnlib/serialize/serialize.h>
#include <snnlib/weight_normalizations/weight_normalization.h>

#include "learning_rule.h"
#include "srm_methods.h"


class Factory;
class BCMRule;

class BCMRuleStat : public Serializable<Protos::BCMRuleStat> {
protected:
    BCMRuleStat() : Serializable<Protos::BCMRuleStat>(EBCMRuleStat) { }
    friend class Factory;
public:
    BCMRuleStat(BCMRule *m);

    void collect(BCMRule *m);

    ProtoPack serialize() {
        Protos::BCMRuleStat *stat = getNewMessage();
        for(auto it=x.begin(); it != x.end(); ++it) {
            Protos::BCMRuleStat::XStat* x_stat = stat->add_x_vals();
            for(auto it_val=it->begin(); it_val != it->end(); ++it_val) {
                x_stat->add_x(*it_val);
            }
        }
        for(auto it=y.begin(); it != y.end(); ++it) {
            stat->add_y(*it);
        }
        for(auto it=p_acc.begin(); it != p_acc.end(); ++it) {
            stat->add_p_acc(*it);
        }
        return ProtoPack({stat});
    }

    virtual void deserialize() {
        Protos::BCMRuleStat * m = getSerializedMessage();
        for(size_t i=0; i<m->x_vals_size(); i++) {
            Protos::BCMRuleStat::XStat x_m = m->x_vals(i);

            vector<double> x_v;
            for(size_t j=0; j<x_m.x_size(); j++) {
                x_v.push_back(x_m.x(j));
            }
            x.push_back(x_v);
        }
        for(size_t i=0; i<m->y_size(); i++) {
            y.push_back(m->y(i));
        }
        for(size_t i=0; i<m->p_acc_size(); i++) {
            p_acc.push_back(m->p_acc(i));
        }
    }
    void reset() {
        p_acc.clear();
        y.clear();
        for(auto it=x.begin(); it != x.end(); ++it) {
            it->clear();
        }

    }
    void print(std::ostream& str) const {
    }
    vector<double> p_acc;
    vector<double> y;
    vector<vector<double>> x;
};



class BCMRule : public LearningRule  {
protected:
    BCMRule() : LearningRule() {
        Serializable::init(EBCMRule);
        stat = nullptr;
        n = nullptr;
    }
    friend class Factory;

public:
    BCMRule(const ConstObj *_c, Neuron *_n, ActFunc *_act_f, WeightNormalization *_wnorm) {
    	init(_c, _n, _act_f, _wnorm);
    }

    void init(const ConstObj *_c, Neuron *_n, ActFunc *_act_f, WeightNormalization *_wnorm) {
        c = castType<BCMRuleC>(_c);
        n = _n;
        p_acc = 0.0;

        stat = nullptr;
        wnorm = _wnorm;

        _act_f->provideRuntime(act_rt);
        if(_wnorm) {
            _wnorm->provideRuntime(wnorm_rt, WeightNormalization::Derivative);
        } else {
            WeightNormalization::provideDefaultRuntime(wnorm_rt);
        }

        Serializable::init(EBCMRule);
        reset();
    }
    void addSynapse(Synapse *s) {
        x.push_back(0.0);
        if(collectStatistics) {
            stat->x.push_back(vector<double>());
        }
    }
    void reset() {
        y = 0.0;
        fill(x.begin(), x.end(), 0.0);
        if(stat) stat->reset();
    }
    void enableCollectStatistics() {
        collectStatistics = true;
        stat = Factory::inst().registerObj<BCMRuleStat>(new BCMRuleStat(this));
    }

    void propagateSynSpike(const SynSpike *sp) {
        if(n->glob_c->getSimTime() >= c->tau_mean) {
            if(fabs(x[sp->syn_id]) < SYN_ACT_TOL) {
                active_synapses.push_back(sp->syn_id);
            }
        }
    }
    void calculateWeightsDynamics()  {
        if(n->glob_c->getSimTime() >= c->tau_mean) {
            wnorm_rt.preModifyMeasure();

            y += (-y + (double)n->fired)/c->tau_y;

            auto it=active_synapses.begin();
            while(it != active_synapses.end()) {
                Synapse *syn = n->syns[*it];

                x[*it] += (-x[*it]+(double)syn->fired)/c->tau_x;

                double threshold = p_acc*p_acc;
                double dw = y * (y - threshold) * x[*it];


                wnorm_rt.modifyWeightDerivative(dw, *it);
                syn->w += dw;


                if(fabs(x[*it]) < SYN_ACT_TOL) {
                    it = active_synapses.erase(it);
                } else {
                    it++;
                }
            }
            if(collectStatistics) {
                stat->collect(this);
            }
        }

        p_acc += (-p_acc+n->fired)/c->tau_mean;

    }
    void provideRuntime(LearningRuleRuntime &rt) {
        rt.calculateWeightsDynamics = MakeDelegate(this, &BCMRule::calculateWeightsDynamics);
        rt.propagateSynSpike = MakeDelegate(this, &BCMRule::propagateSynSpike);
    }


    void deserialize() {
        Protos::BCMRule *mess = getSerializedMessage<Protos::BCMRule>();
        p_acc = mess->p_acc();

        x.clear();
        if(stat) stat->x.clear();
        if(n) {
            for(auto it=n->syns.begin(); it != n->syns.end(); ++it) {
                addSynapse(*it);
            }
        }
        reset();
    }
    ProtoPack serialize() {
        Protos::BCMRule *mess = getNewMessage<Protos::BCMRule>();
        mess->set_p_acc(p_acc);
        return ProtoPack({mess});
    }
    ProtoPack getNew() {
        return ProtoPack({ getNewMessage<Protos::BCMRule>() });
    }
    void saveStat(SerialPack &p) {
        p.push_back(stat);
    }

    void print(std::ostream& str) const { }

    double p_acc;
    vector<double> x;
    double y;
    list<size_t> active_synapses;

    const BCMRuleC *c;
    BCMRuleStat *stat;
    Neuron *n;

};

