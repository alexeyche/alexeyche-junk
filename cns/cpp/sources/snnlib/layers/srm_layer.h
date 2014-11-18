#pragma once

#include "layer.h"

#include "neuron.h"



class SRMNeuron : public Neuron {
protected:
    SRMNeuron() { }
    friend class Factory;
public:
    SRMNeuron(const ConstObj *_c, ActFunc *_act, LearningRule *_lrule, TuningCurve *_tc) {
        init(_c, _act, _lrule, _tc);
    }
    void init(const ConstObj *_c, ActFunc *_act, LearningRule *_lrule, TuningCurve *_tc) {
        Neuron::init(_c, _act, _lrule, _tc);
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
            stat->p.push_back(p);
            stat->u.push_back(u);
            for(size_t syn_i=0; syn_i<syns.size(); syn_i++) {
                stat->syns[syn_i].push_back(syns[syn_i]->x);
            }
        }
    }

    void attachCurrent(const double &I) {
        y = tc->calculateResponse(I);
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




class SRMLayer : public Layer {
protected:
    SRMLayer() { }
    friend class Factory;
public:

    void init(size_t _size, const ConstObj *_c, const NeuronConf &nc, const Constants &glob_c) {
        Layer::init(_size, _c, nc, glob_c);
        CAST_TYPE(SRMLayerC, bc)
        c = cast;
    }

    void calculate() {

    }
    const SRMLayerC *c;
};


