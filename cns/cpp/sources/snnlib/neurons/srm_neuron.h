#pragma once

#include "neuron.h"



class SRMNeuron : public Neuron {
protected:
    SRMNeuron() { }
    friend class Factory;
public:
    SRMNeuron(const ConstObj *_c, const RuntimeGlobals *_glob_c, double _axon_delay) {
        init(_c, _glob_c, _axon_delay);
    }
    void init(const ConstObj *_c, const RuntimeGlobals *_glob_c, double _axon_delay) {
        Neuron::init(_c, _glob_c, _axon_delay);
        c = castType<SRMNeuronC>(bc);
    }

    void propagateSynSpike(const SynSpike *sp) {
        if( fabs(syns[sp->syn_id]->x) < SYN_ACT_TOL ) {
            active_synapses.push_back(syns[sp->syn_id]);
        }
        //cout << "Propagating syns " << sp.syn_id << " (" << syns.size() << ")\n";
        syns[sp->syn_id]->propagateSpike();
    }

    void calculateProbability() {
        y = c->u_rest;
        for(auto it=active_synapses.begin(); it != active_synapses.end(); ++it) {
            Synapse *s = *it;
            y += s->w * s->x;
        }
        p = act->prob(y);
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
        }
        lrule->calculateWeightsDynamics();

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
        str << "SRMNeuron(" << id << ")\n";
        str << "\ty == " << y << ", axon_delay: " << axon_delay << ", synapses\n";
        for(auto it=syns.begin(); it != syns.end(); ++it) {
            str << **it << ", ";
        }
        str << "\n";
    }

    const SRMNeuronC *c;

};


