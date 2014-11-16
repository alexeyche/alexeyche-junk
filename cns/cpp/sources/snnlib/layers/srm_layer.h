#pragma once

#include "layer.h"

#include <snnlib/sim/sim_layer.h>
#include <snnlib/sim/sim_neuron.h>

#include "neuron.h"

static bool synapseNotActive(const Synapse *s) {
    if(fabs(s->x) < 0.0001 ) return true;
    return false;
}

class SRMNeuron : public Neuron, public SimNeuron {
public:
    SRMNeuron(size_t _id, const ConstObj *_c, const ActFunc *_act, const LearningRule *_lrule, const TuningCurve *_tc) 
    	: Neuron(_id, _c, _act, _lrule, _tc) {
		CAST_TYPE(SRMLayerC, Neuron::bc.get())    		
    }

    void calculateProbability() {
        double u = c->u_rest;
        for(auto it=active_synapses.begin(); it != active_synapses.end(); ++it) {
            Synapse *s = *it;
            u += s->w * s->x;
        }
        p = act->prob(u);
    }    
    void calculateDynamics() {
        if(p > getUnif()) {
            fired = 1;
        }
        
        //active_synapses.erase(active_synapses.remove_if(active_synapses.begin(), active_synapses.end(), synapseNotActive), active_synapses.end());
        // to test efficiancy
        auto it=active_synapses.begin();
        while(it != active_synapses.end()) {
            Synapse *s = *it;
            if(fabs(s->x) < 0.0001 ) {
                it = active_synapses.erase(it);
            } else {
                s->x -= s->x/s->c->epsp_delay;
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
            str << *it << ", ";
        }
        str << "\n";
    }
    
    shared_ptr<const SRMLayerC> c;
};




class SRMLayer : public Layer, public SimLayer {
private:    
    SRMLayer() { }

    friend class Factory;
public:
    Neuron* addNeuron(const ConstObj *_c, const ActFunc *_act, const LearningRule *_lrule, const TuningCurve *_tc) {
        return new SRMNeuron(++global_neuron_index, _c,_act,_lrule,_tc);
    }
    void init(size_t _id, size_t _size, const ConstObj *_c, const ActFunc *_act, const LearningRule *_lrule, const TuningCurve *_tc) {
        Layer::init(_id, _size, _c, _act, _lrule, _tc);
    }

    void calculate() {

    }
    
};


