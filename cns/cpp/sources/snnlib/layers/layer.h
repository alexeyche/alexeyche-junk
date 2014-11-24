#pragma once

#include "neuron.h"

static size_t global_layer_index = 0;

#include <snnlib/act_funcs/act_func.h>
#include <snnlib/learning/learning_rule.h>
#include <snnlib/config/constants.h>
#include <snnlib/tuning_curves/tuning_curve.h>

#include "common.h"

#include <snnlib/config/factory.h>


class Layer : public Entity {
protected:
    Layer() {}
    friend class Factory;
public:
    Layer(size_t _size, const NeuronConf &nc, const Constants &c, const RuntimeGlobals *run_glob_c) {
        init(_size, nc, c, run_glob_c);
    }

    virtual void init(size_t _size, const NeuronConf &nc, const Constants &c, const RuntimeGlobals *run_glob_c) {
        id = global_layer_index++;
        N = _size;
        for(size_t ni=0; ni<N; ni++) {
            double axon_delay = sampleDelay(nc.axon_delay_gain, nc.axon_delay_rate);
            Neuron *n = Factory::inst().createNeuron(nc.neuron, c, run_glob_c, axon_delay);

            ActFunc *act = Factory::inst().createActFunc(nc.act_func, c, n);

            LearningRule *lr;
            if(nc.learning_rule.empty()) {
                lr = Factory::inst().createLearningRule("BlankLearningRule", c, nullptr);
            } else {
                lr = Factory::inst().createLearningRule(nc.learning_rule, c, n);
            }
            TuningCurve *tc;
            if(nc.tuning_curve.empty()) {
                tc = Factory::inst().createTuningCurve("BlankTuningCurve", c, N, ni, nullptr);
            } else {
                tc = Factory::inst().createTuningCurve(nc.tuning_curve, c, N, ni, n);
            }

            neurons.push_back(n);
        }
    }



    size_t size() {
        return neurons.size();
    }
    Neuron *operator[](size_t i) {
        return neurons[i];
    }
    void connect(Layer &l_post, const ConnectionConf &conf, const Constants &c) {
        for(size_t ni=0; ni<neurons.size(); ni++) {
            for(size_t nj=0; nj<l_post.N; nj++) {
                if(neurons[ni]->id != l_post[nj]->id) {
                    double prob = getUnif();
                    if( conf.prob > prob ) {
                        double dendrite_delay = sampleDelay(conf.dendrite_delay_gain, conf.dendrite_delay_rate);
                        Synapse *s = Factory::inst().createSynapse(conf.type, c, neurons[ni]->id, 0, dendrite_delay);
                        l_post[nj]->addSynapse(s);
                    }
                }
            }
        }
        for(size_t ni=0; ni<l_post.N; ni++) {
            Neuron *n = l_post.neurons[ni];
            for(size_t con_i=0; con_i<n->syns.size(); con_i++) {
                Synapse *s = n->syns[con_i];
                s->w = conf.weight_per_neuron/((double)n->syns.size());
            }
        }
    }

    void print(std::ostream& str) const {
        for(auto it=neurons.begin(); it!=neurons.end(); ++it) {
            str << **it;
        }
    }

    size_t id;
    size_t N;
    vector< Neuron *> neurons;
};
