#pragma once

#include <snnlib/neurons/neuron.h>
#include <snnlib/serialize/serialize.h>
#include <snnlib/weight_normalizations/weight_normalization.h>

#include "learning_rule.h"


class Factory;
class TripleStdp;

class TripleStdpStat : public Serializable<Protos::TripleStdpStat> {
protected:
    TripleStdpStat() : Serializable<Protos::TripleStdpStat>(ETripleStdpStat) { }
    friend class Factory;
public:
    TripleStdpStat(TripleStdp *m);

    void collect(TripleStdp *m);

    ProtoPack serialize() {
        Protos::TripleStdpStat *stat = getNewMessage();
        
        for(size_t ri=0; ri < r1.size(); ri++) {
            Protos::TripleStdpStat::XTrace* x_trace_stat = stat->add_x_trace_vals();
            
            for(size_t rivi=0; rivi < r1[ri].size(); rivi++) {
                x_trace_stat->add_r1(r1[ri][rivi]);
                if(r2.size()>0) {
                    x_trace_stat->add_r2(r2[ri][rivi]);
                }
                
            }            
        }
        for(auto it=o1.begin(); it != o1.end(); ++it) {
            stat->add_o1(*it);
        }
        for(auto it=o2.begin(); it != o2.end(); ++it) {
            stat->add_o2(*it);
        }
        return ProtoPack({stat});
    }

    virtual void deserialize() {
        Protos::TripleStdpStat * m = getSerializedMessage();
        r1.clear();
        r2.clear();
        for(size_t i=0; i<m->x_trace_vals_size(); i++) {
            Protos::TripleStdpStat::XTrace x_m = m->x_trace_vals(i);

            vector<double> r1_v;
            vector<double> r2_v;
            if(x_m.r2_size()>0) assert(x_m.r1_size() == x_m.r2_size());
            for(size_t j=0; j<x_m.r1_size(); j++) {
                r1_v.push_back(x_m.r1(j));
            }
            for(size_t j=0; j<x_m.r2_size(); j++) {
                r2_v.push_back(x_m.r2(j));
            }
            r1.push_back(r1_v);
            if(r2_v.size()>0) r2.push_back(r2_v);
        }
        for(size_t i=0; i<m->o1_size(); i++) {
            o1.push_back(m->o1(i));
        }
        for(size_t i=0; i<m->o2_size(); i++) {
            o2.push_back(m->o2(i));
        }
    }
    
    void reset() {
        o1.clear();
        o2.clear();
        for(auto it = r1.begin(); it != r1.end(); ++it) {
            it->clear();
        }
        for(auto it = r2.begin(); it != r2.end(); ++it) {
            it->clear();
        }
    }

    void print(std::ostream& str) const {
    }
    vector<double> o1;
    vector<double> o2;
    vector<vector<double>> r1;
    vector<vector<double>> r2;
};



class TripleStdp : public LearningRule  {
protected:
    TripleStdp() : LearningRule() {
        Serializable::init(ETripleStdp);
    }
    friend class Factory;
public:    
    TripleStdp(const ConstObj *_c, Neuron *_n, ActFunc *_act_f, WeightNormalization *_wnorm) : LearningRule() {
    	init(_c, _n, _act_f, _wnorm);
    }

    void init(const ConstObj *_c, Neuron *_n, ActFunc *_act_f, WeightNormalization *_wnorm) {
        c = castType<TripleStdpC>(_c);
        n = _n;
        wnorm = _wnorm;

        o1 = 0.0;
        o2 = 0.0;
        r1.resize(n->syns.size());
        fill(r1.begin(), r1.end(), 0.0);
        if(!c->minimal_model) {
            r2.resize(n->syns.size());
            fill(r2.begin(), r2.end(), 0.0);
        }
        
        stat = nullptr;
        _act_f->provideRuntime(act_rt);
        WeightNormalization::provideDefaultRuntime(wnorm_rt);
        if(_wnorm) {
            _wnorm->provideRuntime(wnorm_rt, WeightNormalization::LtpLtd);
        }
        Serializable::init(ETripleStdp);
    }
    void addSynapse(Synapse *s) {
        r1.push_back(0.0);        
        if(!c->minimal_model) r2.push_back(0.0);
        if(collectStatistics) {
            stat->r1.push_back(vector<double>());
            if(!c->minimal_model) stat->r2.push_back(vector<double>());
        }
    }

    void enableCollectStatistics() {
        collectStatistics = true;
        stat = Factory::inst().registerObj<TripleStdpStat>(new TripleStdpStat(this));
    }
    void reset() {
        o1 = 0.0;
        o2 = 0.0;
        fill(r1.begin(), r1.end(), 0.0);
        fill(r2.begin(), r2.end(), 0.0);
        if(stat) stat->reset();
    }

    void propagateSynSpike_Min(const SynSpike *sp) {
        if (fabs(r1[sp->syn_id]) < SYN_ACT_TOL) {
            active_synapses.push_back(sp->syn_id);
        }
        r1[sp->syn_id] += 1;
    }


    void calculateWeightsDynamics_Min()  {
        wnorm_rt.preModifyMeasure();

        if(n->fired) {
            o1 += 1;
        }

        auto it=active_synapses.begin();
        while(it != active_synapses.end()) {
            Synapse *syn = n->syns[*it];
            
            double dw = c->learning_rate * ( n->fired   * wnorm_rt.ltpMod(syn->w) * c->a3_plus  * r1[*it] *  o2 - 
                                             syn->fired * wnorm_rt.ltdMod(syn->w) * c->a2_minus * o1);
                                             

            wnorm_rt.modifyWeightDerivative(dw, *it);
            syn->w += dw;

            r1[*it] += - r1[*it]/c->tau_plus;
            
           
            if (fabs(r1[*it]) < SYN_ACT_TOL) {
                it = active_synapses.erase(it);
            } else {
                it++;
            }
        }
        if(n->fired) {
            o2 += 1;
        }
        o1 += - o1/c->tau_minus;
        o2 += - o2/c->tau_y;

        if(collectStatistics) {
            stat->collect(this);
        }
    }

    void propagateSynSpike_Full(const SynSpike *sp) {
        if ((fabs(r1[sp->syn_id]) < SYN_ACT_TOL) || (fabs(r2[sp->syn_id]) < SYN_ACT_TOL)) {
            active_synapses.push_back(sp->syn_id);
        }
        r1[sp->syn_id] += 1;
    }

    void calculateWeightsDynamics_Full()  {
        wnorm_rt.preModifyMeasure();

        if(n->fired) {
            o1 += 1;
        }

        auto it=active_synapses.begin();
        while(it != active_synapses.end()) {
            Synapse *syn = n->syns[*it];
            
            double dw = c->learning_rate * ( n->fired   * wnorm_rt.ltpMod(syn->w) * r1[*it] * (c->a2_plus  + c->a3_plus  * o2     ) - 
                                             syn->fired * wnorm_rt.ltdMod(syn->w) * o1      * (c->a2_minus + c->a3_minus * r2[*it]));
                                             

            wnorm_rt.modifyWeightDerivative(dw, *it);
            syn->w += dw;

            r1[*it] += - r1[*it]/c->tau_plus;
            
            if(syn->fired) {
                r2[*it] += 1;    
            }

            r2[*it] += - r2[*it]/c->tau_x;

            if((fabs(r1[*it]) < SYN_ACT_TOL) || (fabs(r2[*it]) < SYN_ACT_TOL)) {
                it = active_synapses.erase(it);
            } else {
                it++;
            }
        }
        if(n->fired) {
            o2 += 1;
        }
        o1 += - o1/c->tau_minus;
        o2 += - o2/c->tau_y;

        if(collectStatistics) {
            stat->collect(this);
        }
    }

    void provideRuntime(LearningRuleRuntime &rt) {
        if(c->minimal_model) {
            rt.calculateWeightsDynamics = MakeDelegate(this, &TripleStdp::calculateWeightsDynamics_Min);
            rt.propagateSynSpike = MakeDelegate(this, &TripleStdp::propagateSynSpike_Min);
        } else {
            rt.calculateWeightsDynamics = MakeDelegate(this, &TripleStdp::calculateWeightsDynamics_Full);
            rt.propagateSynSpike = MakeDelegate(this, &TripleStdp::propagateSynSpike_Full);
        }
    }

    void deserialize() {
        Protos::TripleStdp *mess = getSerializedMessage<Protos::TripleStdp>();
        o1 = mess->o1();
        o2 = mess->o2();
        r1.clear();
        if(r2.size()>0) {
            r2.clear();
            assert(mess->r1_size() == mess->r2_size());
        }
        for(size_t ri=0; ri < mess->r1_size(); ri++) {
            r1.push_back(mess->r1(ri));
        }
        for(size_t ri=0; ri < mess->r2_size(); ri++) {
            r2.push_back(mess->r2(ri));
        }
    }
    ProtoPack serialize() {
        Protos::TripleStdp *mess = getNewMessage<Protos::TripleStdp>();
        mess->set_o1(o1);
        mess->set_o2(o2);
        if(r2.size()>0) assert(r1.size() == r2.size());
        for(size_t ri=0; ri < r1.size(); ri++) {
            mess->add_r1(r1[ri]);
            if(ri<r2.size()) mess->add_r2(r2[ri]);            
        }
        return ProtoPack({mess});
    }
    ProtoPack getNew() {
        return ProtoPack({ getNewMessage<Protos::TripleStdp>() });
    }
    void saveStat(SerialPack &p) {
        p.push_back(stat);
    }

    void print(std::ostream& str) const { }


    double o1;
    double o2;
    vector<double> r1;
    vector<double> r2;
    
    list<size_t> active_synapses;


    const TripleStdpC *c;
    TripleStdpStat *stat;
    Neuron *n;
};

