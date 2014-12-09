#pragma once

#include "neuron.h"

#include <snnlib/util/fastapprox/fastexp.h>

class SRMNeuron : public Neuron {
protected:
    SRMNeuron() { }
    friend class Factory;
public:
    SRMNeuron(const ConstObj *_c, size_t _local_id,  const RuntimeGlobals *_glob_c, double _axon_delay) {
        init(_c, _local_id, _glob_c, _axon_delay);
    }
    void init(const ConstObj *_c, size_t _local_id,  const RuntimeGlobals *_glob_c, double _axon_delay) {
        Neuron::init(_c, _local_id, _glob_c, _axon_delay);
        Serializable::init(ESRMNeuron);
        c = castType<SRMNeuronC>(bc);
    }

    void propagateSynSpike(const SynSpike *sp) {
        if( fabs(syns[sp->syn_id]->x) < SYN_ACT_TOL ) {
            active_synapses.push_back(sp->syn_id);
        }
        //cout << "Propagating syns " << sp.syn_id << " (" << syns.size() << ")\n";
        syns[sp->syn_id]->propagateSpike();
    }

    void calculateProbability() {
        y = 0.0;
        for(auto it=active_synapses.begin(); it != active_synapses.end(); ++it) {
            Synapse *s = syns[*it];
            y += s->w * s->x;
        }
        y = c->u_rest + y * weight_factor;
        M = fastexp(-gr);
        p = act->prob(y) * M;
        if(collectStatistics) {
            stat->collect(this);
        }
    }

    void attachCurrent(const double &I) {
        tc->calculateResponse(I);
    }

    void calculateDynamics() {
        if(p > getUnif()) {
            fired = 1;
            gr += c->amp_refr;
        }
        lrule->calculateWeightsDynamics();

        auto it=active_synapses.begin();
        while(it != active_synapses.end()) {
            Synapse *s = syns[*it];
            if(fabs(s->x) < SYN_ACT_TOL) {
                it = active_synapses.erase(it);
            } else {
                s->x -= s->x/s->c->epsp_decay;
                s->fired = 0;
                ++it;
            }
        }
        gr += - gr/c->tau_refr;
        if(rmod) rmod->modulateReward();
    }
    ProtoPack serialize() {
        return Neuron::serialize();
    }
    void deserialize() {
        Neuron::deserialize();
    }

    void print(std::ostream& str) const {
        str << "SRMNeuron(" << id << ")\n";
        str << "\ty == " << y << ", p == " << p << ", M == " << M << ", gr == " << gr << ", fired == " << fired << ", axon_delay: " << axon_delay << ", synapses\n";
        for(auto it=syns.begin(); it != syns.end(); ++it) {
            str << **it << ", ";
        }
        str << "lrule: " << lrule->getName() << "\n";
        str << "\n";
    }

    const SRMNeuronC *c;

};


