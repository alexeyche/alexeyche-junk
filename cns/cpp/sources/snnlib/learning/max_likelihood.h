#pragma once

#include "learning_rule.h"

#include <snnlib/neurons/neuron.h>
#include <snnlib/learning/srm_methods.h>
#include <snnlib/serialize/serialize.h>
#include <snnlib/protos/model.pb.h>

class Factory;
class MaxLikelihood;

class MaxLikelihoodStat : public Serializable<Protos::MaxLikelihoodStat> {
protected:
    MaxLikelihoodStat() : Serializable<Protos::MaxLikelihoodStat>(EMaxLikelihoodStat) { }
    friend class Factory;
public:
    MaxLikelihoodStat(MaxLikelihood *m);

    void collect(MaxLikelihood *m);

    ProtoPack serialize() {
        Protos::MaxLikelihoodStat *stat = getNewMessage();
        for(auto it=eligibility_trace.begin(); it != eligibility_trace.end(); ++it) {
            Protos::MaxLikelihoodStat::ElTrace* syn_stat = stat->add_el_traces();
            for(auto it_val=it->begin(); it_val != it->end(); ++it_val) {
                syn_stat->add_x(*it_val);
            }
        }
        return ProtoPack({stat});
    }

    virtual void deserialize() {
        Protos::MaxLikelihoodStat * m = getSerializedMessage();
        for(size_t i=0; i<m->el_traces_size(); i++) {
            Protos::MaxLikelihoodStat::ElTrace syn_m = m->el_traces(i);

            vector<double> x_v;
            for(size_t j=0; j<syn_m.x_size(); j++) {
                x_v.push_back(syn_m.x(j));
            }
            eligibility_trace.push_back(x_v);
        }
    }

    void print(std::ostream& str) const {
    }

    vector<vector<double>> eligibility_trace;
};




class MaxLikelihood : public LearningRule  {
protected:
    MaxLikelihood() {
        Serializable::init(EMaxLikelihood);
    }
    friend class Factory;

public:
    MaxLikelihood(const MaxLikelihoodC *_c, Neuron *_n) {
        init(_c, _n);
    }
    void init(const ConstObj *_c, Neuron *_n) {
        c = castType<MaxLikelihoodC>(_c);
        n = _n;
        eligibility_trace.resize(n->syns.size());
        fill(eligibility_trace.begin(), eligibility_trace.end(), 0.0);

        Serializable::init(EMaxLikelihood);

        stat = nullptr;
    }
    void addSynapse(Synapse *s) {
        eligibility_trace.push_back(0.0);
        if(collectStatistics) {
            stat->eligibility_trace.push_back(vector<double>());
        }
    }
    void enableCollectStatistics() {
        collectStatistics = true;
        stat = Factory::inst().registerObj<MaxLikelihoodStat>(new MaxLikelihoodStat(this));
    }
    void saveStat(SerialPack &p) {
        p.push_back(stat);
    }
    void calculateWeightsDynamics()  {
        for(auto it=n->active_synapses.begin(); it != n->active_synapses.end(); ++it) {
            Synapse *syn = n->syns[*it];
            double dw;
            if(c->input_target) {
                dw = SRMMethods::dLLH_dw_given_Y(n, syn, n->glob_c->inputNeuronsFiring(n->id));
            } else {
                dw = SRMMethods::dLLH_dw(n, syn);
            }
            if(fabs(eligibility_trace[*it]) < SYN_ACT_TOL) {
                active_synapses.push_back(*it);
            }
            eligibility_trace[*it] += dw;
            if(std::isnan(dw)) {
                cout << "Found nan dw:\n";
                double fired = n->fired;
                if(c->input_target) {
                    fired = n->glob_c->inputNeuronsFiring(n->id);
                }
                cout << n->act->probDeriv(n->y) << "/(" << n->p  << "/" << n->M << ") * (" << fired << "-" << n->p << ") * " << syn->x << " == ";
                if(c->input_target) {
                    cout << SRMMethods::dLLH_dw_given_Y(n, syn, n->glob_c->inputNeuronsFiring(n->id)) << "\n";
                } else {
                    cout << SRMMethods::dLLH_dw(n, syn) << "\n";
                }
                cout << *n;
                terminate();
            }
        }
        const Reward* rew = n->getReward();

        auto it=active_synapses.begin();
        while(it != active_synapses.end()) {
            Synapse *syn = n->syns[*it];
            if(fabs(eligibility_trace[*it]) < SYN_ACT_TOL) {
                it = active_synapses.erase(it);
            } else {
                double dw = c->learning_rate * eligibility_trace[*it];
                if(rew) {
                    dw *= (rew->r - rew->mean_r);
                }
                syn->w += dw;
                eligibility_trace[*it] -= eligibility_trace[*it]/c->tau_el;
                ++it;
            }

        }
        if(collectStatistics) {
            stat->collect(this);
        }
    }
    void deserialize() {
    }
    ProtoPack serialize() {
        return getNew();
    }
    ProtoPack getNew() {
        return ProtoPack();
    }

    void print(std::ostream& str) const { }

    vector<double> eligibility_trace;

    list<size_t> active_synapses;

    const MaxLikelihoodC *c;
    Neuron *n;

    MaxLikelihoodStat *stat;
};

