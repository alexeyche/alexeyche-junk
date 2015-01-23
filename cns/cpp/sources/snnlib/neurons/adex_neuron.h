#pragma once

#include "neuron.h"
#include <snnlib/protos/stat.pb.h>

class AdExNeuron;

class AdExNeuronStat : public NeuronStat  {
protected:
    AdExNeuronStat() {
        init(EAdExNeuronStat);
    }
    friend class Factory;
public:
    AdExNeuronStat(Neuron *n) : NeuronStat(n) {
        init(EAdExNeuronStat);
    }
    virtual void collect(AdExNeuron *n);

    AdExNeuronStat(const AdExNeuronStat &another);
    ProtoPack serialize();

    void deserialize() {
        NeuronStat::deserialize();
        Protos::AdExNeuronStat * m = getSerializedMessage<Protos::AdExNeuronStat>(1);
        for(size_t i=0; i<m->a_size(); i++) {
            a.push_back(m->a(i));
        }
    }
    ProtoPack getNew() {
        return ProtoPack({ getNewMessage(), getNewMessage<Protos::AdExNeuronStat>() });
    }
    void print(std::ostream& str) const {}
    void reset() {
        NeuronStat::reset();
        a.clear();
    }
    vector<double> a;
};

class AdExNeuron : public Neuron {
protected:
    AdExNeuron() {
        adex_stat = nullptr;
    }
    friend class Factory;
public:
    AdExNeuron(const ConstObj *_c, size_t _local_id, const RuntimeGlobals *_glob_c, double _axon_delay) {
        init(_c, _local_id, _glob_c, _axon_delay);

    }
    void init(const ConstObj *_c, size_t _local_id, const RuntimeGlobals *_glob_c, double _axon_delay) {
        Serializable::init(EAdExNeuron);
        adex_stat = nullptr;
        Neuron::init(_c, _local_id, _glob_c, _axon_delay);
        c = castType<AdExNeuronC>(bc);
    }

    void enableCollectStatistics() {
        collectStatistics = true;
        adex_stat = Factory::inst().registerObj<AdExNeuronStat>(new AdExNeuronStat(this));
    }
    void reset() {
        Neuron::reset();
        I = 0.0;

        a = 0.0;
        refr = 0.0;
    }

    // Runtime
    void propagateSynSpike(const SynSpike *sp) {
        Synapse *s = syns[sp->syn_id];
        if( fabs(s->x) < SYN_ACT_TOL ) {
            active_synapses.push_back(sp->syn_id);
        }

        syns_rt[sp->syn_id].propagateSpike();
        lrule_rt.propagateSynSpike(sp);
    }

    void attachCurrent(const double &I_attach) {
        I = tc_rt.calculateResponse(I_attach);
        //cout << "\t response: " << y << "\n";
    }

    void calculateProbability() {
        if(refr > 0.000001) {
            refr -= glob_c->Dt();
        } else {
            double dV = I - c->gL * ( y - c->EL );
            I = 0.0;
            for(auto it=active_synapses.begin(); it != active_synapses.end(); ++it) {
                Synapse *s = syns[*it];
                dV += s->w * s->x;
            }
            if(fabs(c->slope) > 0.000001) {
                dV += c->gL * c->slope * exp( (y - c->u_tr)/c->slope );
            }
            double da = c->a * ( y - c->EL ) - a;

            y += glob_c->Dt() * ( dV/c->C );
            a += glob_c->Dt() * ( da/c->tau_a );

            p = act_rt.prob(y);
        }
        if(collectStatistics) {
            adex_stat->collect(this);
        }
    }



    void calculateDynamics() {
        double coin = getUnif();
        if(p > coin) {
            // cout << "\t\t Neuron " << id << " fired: " << p << " > " << coin << "\n";
            fired = 1;
            y = c->EL;
            a += c->b;
            refr = c->t_ref;
            p = 0.0;
        }
        lrule_rt.calculateWeightsDynamics();

        auto it=active_synapses.begin();
        while(it != active_synapses.end()) {
            Synapse *s = syns[*it];
            if(fabs(s->x) < SYN_ACT_TOL) {
                it = active_synapses.erase(it);
            } else {
                syns_rt[*it].calculateDynamics();
                ++it;
            }
        }
        rmod_rt.modulateReward();
    }
    void provideRuntime(NeuronRuntime &rt) {
        rt.attachCurrent = MakeDelegate(this, &AdExNeuron::attachCurrent);
        rt.calculateDynamics = MakeDelegate(this, &AdExNeuron::calculateDynamics);
        rt.calculateProbability = MakeDelegate(this, &AdExNeuron::calculateProbability);
        rt.propagateSynSpike = MakeDelegate(this, &AdExNeuron::propagateSynSpike);
    }
    ProtoPack serialize() {
        return Neuron::serialize();
    }
    void deserialize() {
        Neuron::deserialize();
    }

    void saveStat(SerialPack &p) {
        if(adex_stat) {
            p.push_back(adex_stat);
        }
        //lrule->saveStat(p);
    }

    void print(std::ostream& str) const {
        str << "AdExNeuron(" << id << ")\n";
        str << "\ty == " << y << ", axon_delay: " << axon_delay << ", synapses\n";
        for(auto it=syns.begin(); it != syns.end(); ++it) {
            str << **it << ", ";
        }
        str << "\n";
    }


    const AdExNeuronC *c;

    double a;
    double refr;
    double I;
    AdExNeuronStat *adex_stat;
};

