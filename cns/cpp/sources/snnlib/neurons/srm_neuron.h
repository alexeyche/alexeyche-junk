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
        Synapse *s = syns[sp->syn_id];
        if( fabs(s->x) < SYN_ACT_TOL ) {
            active_synapses.push_back(sp->syn_id);
        }
        s->propagateSpike();
        lrule_rt.propagateSynSpike(sp);
    }

    void attachCurrent(const double &I) {
        y = tc_rt.calculateResponse(I);
    }

    void calculateProbability() {
        y = 0.0;
        for(auto it=active_synapses.begin(); it != active_synapses.end(); ++it) {
            Synapse *s = syns[*it];
            y += s->getCurrent() * s->w;
        }
        y = c->u_rest + y;
        M = fastexp(-(gr+ga));
        p = act_rt.prob(y) * M;
        if(collectStatistics) {
            stat->collect(this);
        }
    }


    void calculateDynamics() {
        if(p > getUnif()) {
            fired = 1;
            gr += c->amp_refr;
            ga += c->amp_adapt;
        }
        lrule_rt.calculateWeightsDynamics();

        auto it=active_synapses.begin();
        while(it != active_synapses.end()) {
            Synapse *s = syns[*it];
            if(fabs(s->x) < SYN_ACT_TOL) {
                it = active_synapses.erase(it);
            } else {
                s->calculateDynamics();
                ++it;
            }
        }
        gr += - gr/c->tau_refr;
        ga += - ga/c->tau_adapt;
        rmod_rt.modulateReward();
    }
    void provideRuntime(NeuronRuntime &rt) {
        rt.attachCurrent = MakeDelegate(this, &SRMNeuron::attachCurrent);
        rt.calculateDynamics = MakeDelegate(this, &SRMNeuron::calculateDynamics);
        rt.calculateProbability = MakeDelegate(this, &SRMNeuron::calculateProbability);
        rt.propagateSynSpike = MakeDelegate(this, &SRMNeuron::propagateSynSpike);
    }
    ProtoPack serialize() {
        return Neuron::serialize();
    }
    void deserialize() {
        Neuron::deserialize();
    }

    void print(std::ostream& str) const {
        str << "SRMNeuron(" << id << ")\n";
        str << "\ty == " << y << ", p == " << p << ", M == " << M << ", gr == " << gr << ", ga == " << ga << ", fired == " << fired << ", axon_delay: " << axon_delay << ", synapses\n";
        for(auto it=syns.begin(); it != syns.end(); ++it) {
            str << **it << ", ";
        }
        str << "lrule: " << lrule->getName() << "\n";
        str << "\n";
    }

    const SRMNeuronC *c;

};


