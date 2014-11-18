#pragma once

#include "layer.h"

#include "neuron.h"



class SRMNeuron : public Neuron {
protected:
    SRMNeuron() { }
    friend class Factory;    
public:
    SRMNeuron(const ConstObj *_c) {
        init(_c);
    }
    void init(const ConstObj *_c) {
        Neuron::init(_c);
        CAST_TYPE(SRMNeuronC, bc)
        c = cast;

    }

    void propagateSynSpike(const SynSpike &sp) {
        if( fabs(syns[sp.syn_id]->x) < SYN_ACT_TOL ) {
            active_synapses.push_back(syns[sp.syn_id]);
        }
        syns[sp.syn_id]->propagateSpike();
    }

    void calculateProbability() {
        double u = c->u_rest + y;
        for(auto it=active_synapses.begin(); it != active_synapses.end(); ++it) {
            Synapse *s = *it;
            u += s->w * s->x;
        }
        p = act->prob(u);
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
            cout << "Neuron " << id << " got a spike\n";
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
        str << "SRMNeuron(" << id << ")\n";
        str << "\ty == " << y;
        str << "\tsynapses: \n";
        for(auto it=syns.begin(); it != syns.end(); ++it) {
            str << **it << ", ";
        }
        str << "\n";
    }

    const SRMNeuronC *c;

};


