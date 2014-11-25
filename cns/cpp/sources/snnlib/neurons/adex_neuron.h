#pragma once

#include "neuron.h"
#include <snnlib/protos/stat.pb.h>

class AdExNeuron;



class AdExNeuronStat : public Serializable {
public:
    AdExNeuronStat() : Serializable(EAdExNeuronStat) {
    }
    void collect(AdExNeuron *n);

    AdExNeuronStat(const AdExNeuronStat &another);
    virtual Protos::AdExNeuronStat *serialize();
    virtual void deserialize() {
        Protos::AdExNeuronStat * m = castSerializableType<Protos::AdExNeuronStat>(serialized_message);
        for(size_t i=0; i<m->a_size(); i++) {
            a.push_back(m->a(i));
        }
    }
    virtual Protos::AdExNeuronStat* getNew(google::protobuf::Message* m = nullptr) {
        return getNewSerializedMessage<Protos::AdExNeuronStat>(m);
    }

    void print(std::ostream& str) const {}

    vector<double> a;
};

class AdExNeuron : public Neuron {
protected:
    AdExNeuron() { }
    friend class Factory;
public:
    AdExNeuron(const ConstObj *_c, const RuntimeGlobals *_glob_c, double _axon_delay) {
        init(_c, _glob_c, _axon_delay);
    }
    void init(const ConstObj *_c, const RuntimeGlobals *_glob_c, double _axon_delay) {
        Neuron::init(_c, _glob_c, _axon_delay);
        CAST_TYPE(AdExNeuronC, bc)
        c = cast;

        a = 0.0;
        refr = 0.0;

        adex_stat = nullptr;
    }
    ~AdExNeuron() {
        if(adex_stat) {
            delete adex_stat;
        }
    }

    // vector<string> getDependentConstantsNames() {
    //     vector<string> v;
    //     v.push_back("Global");
    //     return v;
    // }

    // void setDependentConstants(const vector<const ConstObj*> &constants) {
    //     glob_c = dynamic_cast<const GlobalC*>(constants[0]);
    //     if(!glob_c) {
    //         cerr << "Error while getting dependent constants for AdExNeuron\n";
    //         terminate();
    //     }
    // }

    void enableCollectStatistics() {
        Neuron::enableCollectStatistics();
        collectStatistics = true;
        adex_stat = new AdExNeuronStat();
    }
    // Runtime
    void propagateSynSpike(const SynSpike *sp) {
        if( fabs(syns[sp->syn_id]->x) < SYN_ACT_TOL ) {
            active_synapses.push_back(syns[sp->syn_id]);
        }
        syns[sp->syn_id]->propagateSpike();
    }

    void calculateProbability() {
        if(refr > 0.000001) {
            refr -= glob_c->Dt();
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

            y += glob_c->Dt() * ( dV/c->C );
            a += glob_c->Dt() * ( da/c->tau_a );

            p = act->prob(y);
        }
        if(collectStatistics) {
            Neuron::stat->collect(this);
            adex_stat->collect(this);
        }
    }

    void attachCurrent(const double &I) {
        tc->calculateResponse(I);
    }
    void provideDelegates(RunTimeDelegates &rtd) {
        rtd.input_dg.push_back(MakeDelegate(this, &AdExNeuron::attachCurrent));
        rtd.state_dg.push_back(MakeDelegate(this, &AdExNeuron::calculateProbability));
        rtd.state_dg.push_back(MakeDelegate(this, &AdExNeuron::calculateDynamics));
    }

    void calculateDynamics() {
        if(p > getUnif()) {
            fired = 1;
            y = c->EL;
            a += c->b;
            refr = c->t_ref;
            p = 0.0;
            //cout <<"fire\n";
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
    void saveStat(SerialPack &p) {
        p.push_back(SerialFamily({Neuron::stat, adex_stat}));
        lrule->saveStat(p);
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
    AdExNeuronStat *adex_stat;
};

