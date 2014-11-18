#pragma once

#include "neuron.h"
#include <snnlib/protos/adex_stat.pb.h>

class AdExNeuron;

class AdExNeuronStat : public Serializable {
protected:
    AdExNeuronStat() : Serializable(EAdExNeuronStat), ns(nullptr) { }
    friend class SerializableFactory;    
public:    
    AdExNeuronStat(Neuron *n) : Serializable(EAdExNeuronStat) {
        ns = new NeuronStat(n);
    }
    ~AdExNeuronStat() {
        if(ns) delete ns;
    }

    void collect(AdExNeuron *n);

    AdExNeuronStat(const AdExNeuronStat &another) : Serializable(EAdExNeuronStat), ns(another.ns), a(another.a) {
        copyFrom(another);
    }
    virtual Protos::AdExNeuronStat *serialize() {
        Protos::AdExNeuronStat *stat = getNew();

    
    }
    virtual void deserialize() {
        cerr << "Why you need that?\n";
        terminate();
    }
    virtual Protos::AdExNeuronStat* getNew(google::protobuf::Message* m = nullptr) {
        return getNewSerializedMessage<Protos::AdExNeuronStat>(m);
    }
    void print(std::ostream& str) const {
    }

    NeuronStat *ns;

    vector<double> a;
};

class AdExNeuron : public Neuron {
protected:
    AdExNeuron() { }
    friend class Factory;    
public:
    AdExNeuron(const ConstObj *_c) {
        init(_c);
    }
    ~AdExNeuron() {
        if(collectStatistics) {
            delete adex_stat;
        }
    }
    void init(const ConstObj *_c) {
        Neuron::init(_c);
        CAST_TYPE(AdExNeuronC, bc)
        c = cast;
        
        a = 0.0;
        refr = 0.0;

        adex_stat = nullptr;
    }
    void enableCollectStatistics() {
        collectStatistics = true;
        adex_stat = new AdExNeuronStat(this);
    }
    void propagateSynSpike(const SynSpike &sp) {
        if( fabs(syns[sp.syn_id]->x) < SYN_ACT_TOL ) {
            active_synapses.push_back(syns[sp.syn_id]);
        }
        syns[sp.syn_id]->propagateSpike();
    }

    void calculateProbability() {
        if(fabs(refr) > 0.000001) {
            refr -= 1;
        } else {
            double dV = c->u_rest + y;
            for(auto it=active_synapses.begin(); it != active_synapses.end(); ++it) {
                Synapse *s = *it;
                dV += s->w * s->x;
            }
            dV -= - c->gL * ( y - c->EL );
            if(c->slope > 0.0) {
                dV += c->gL * c->slope * exp( (y - c->u_tr)/c->slope );
            }
            double da = c->a * ( y - c->EL ) - a; 
            
            y += dV/c->C;
            a += da/c->tau_a;

            p = act->prob(y);
        }
        if(collectStatistics) {
            adex_stat->collect(this);
        }
    }

    void attachCurrent(const double &I) {
        tc->calculateResponse(I);
    }

    void calculateDynamics() {
        if(p > getUnif()) {
            fired = 1;
            cout << "AdExNeuron " << id << " got a spike\n";
            y = c->EL;
            a += c->b;
            refr = c->t_ref;
            p = 0.0;
        }

        auto it=active_synapses.begin();
        while(it != active_synapses.end()) {
            Synapse *s = *it;
            if(fabs(s->x) < SYN_ACT_TOL) {
                it = active_synapses.erase(it);
            } else {
                s->x -= s->x/s->c->epsp_decay;
                s->fired = 0;
                ++it;
            }
        }


    }
    void print(std::ostream& str) const {
        str << "AdExNeuron(" << id << ")\n";
        str << "\ty == " << y;
        str << "\tsynapses: \n";
        for(auto it=syns.begin(); it != syns.end(); ++it) {
            str << **it << ", ";
        }
        str << "\n";
    }

    const AdExNeuronC *c;

    double a;
    double refr;
    AdExNeuronStat *adex_stat;
};

