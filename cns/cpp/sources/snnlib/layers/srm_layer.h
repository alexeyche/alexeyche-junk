#pragma once

#include "layer.h"

#include "neuron.h"

static bool synapseNotActive(const Synapse *s) {
    if(fabs(s->x) < 0.0001 ) return true;
    return false;
}

class SRMNeuron : public Neuron {
protected:
    SRMNeuron() { }
    friend class Factory;
public:
    SRMNeuron(const ConstObj *_c, ActFunc *_act, LearningRule *_lrule, TuningCurve *_tc) {
        init(_c, _act, _lrule, _tc);
		CAST_TYPE(SRMNeuronC, bc)
    }
    void init(const ConstObj *_c, ActFunc *_act, LearningRule *_lrule, TuningCurve *_tc) {
        Neuron::init(_c, _act, _lrule, _tc);
    }

    void propagateSynSpike(const SynSpike &sp) {
        if( fabs(syns[sp.syn_id]->x) < SYN_ACT_TOL ) {
            active_synapses.push_back(syns[sp.syn_id]);
        }
        syns[sp.syn_id]->propagateSpike();
    }

    void calculateProbability() {
        double u = 0.0; //= c->u_rest + y;
        for(auto it=active_synapses.begin(); it != active_synapses.end(); ++it) {
            Synapse *s = *it;
            u += s->w * s->x;
        }
        p = act->prob(u);
    }

    void attachCurrent(const double &I) {
        y = tc->calculateResponse(I);
    }
    void calculateDynamics() {
        if(p > getUnif()) {
            fired = 1;
            cout << "Neuron " << id << " got a spike\n";
        }

        //active_synapses.erase(active_synapses.remove_if(active_synapses.begin(), active_synapses.end(), synapseNotActive), active_synapses.end());
        // to test efficiancy
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




class SRMLayer : public Layer {
protected:
    SRMLayer() { }
    friend class Factory;
public:

    void init(size_t _size, const ConstObj *_c, const NeuronConf &nc, const Constants &glob_c) {
        Layer::init(_size, _c, nc, glob_c);
        CAST_TYPE(SRMLayerC, bc)
    }

    void calculate() {

    }
    const SRMLayerC *c;
};


