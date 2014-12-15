#pragma once

#include <snnlib/base.h>
#include <snnlib/neurons/neuron.h>
#include <snnlib/act_funcs/act_func.h>
#include <snnlib/learning/learning_rule.h>
#include <snnlib/config/constants.h>
#include <snnlib/tuning_curves/tuning_curve.h>
#include <snnlib/config/factory.h>
#include <snnlib/serialize/proto_rw.h>

static size_t global_layer_index = 0;

class Layer : public Printable {
protected:
    Layer() {}
    friend class Factory;
public:
    Layer(size_t _size, bool _wta, const NeuronConf &nc, const Constants &c, RuntimeGlobals *run_glob_c) {
        init(_size, _wta, nc, c, run_glob_c);
    }

    virtual void init(size_t _size, bool _wta, const NeuronConf &nc, const Constants &c, RuntimeGlobals *run_glob_c) {
        id = global_layer_index++;
        N = _size;
        neuron_conf = &nc;
        glob_c = run_glob_c;
        p_wta = 0.0;
        wta = _wta;
        for(size_t ni=0; ni<N; ni++) {
            double axon_delay = sampleDelay(nc.axon_delay_gain, nc.axon_delay_rate);

            Neuron *n = Factory::inst().createNeuron(nc.neuron, ni, c, run_glob_c, axon_delay);
            n->setActFunc(Factory::inst().createActFunc(nc.act_func, c, n));
            if(!nc.reward_modulation.empty()) {
                n->setRewardModulation(Factory::inst().createRewardModulation(nc.reward_modulation, c, n, run_glob_c));
            }
            WeightNormalization *wnorm = nullptr;
            if(!nc.weight_normalization.empty()) {
                wnorm = Factory::inst().createWeightNormalization(nc.weight_normalization, c, n);
            }
            if(nc.learning_rule.empty()) {
                n->setLearningRule(Factory::inst().createLearningRule("BlankLearningRule", c, nullptr, nullptr));
            } else {
                n->setLearningRule(Factory::inst().createLearningRule(nc.learning_rule, c, n, wnorm));
            }
            if(nc.tuning_curve.empty()) {
                n->setTuningCurve(Factory::inst().createTuningCurve("BlankTuningCurve", c, N, ni, nullptr));
            } else {
                n->setTuningCurve(Factory::inst().createTuningCurve(nc.tuning_curve, c, N, ni, n));
            }

            neurons.push_back(n);
        }
    }

    void saveModel(ProtoRw &rw) {
        for(auto it = neurons.begin(); it != neurons.end(); ++it) {
            (*it)->saveModel(rw);
        }
    }

    void loadModel(ProtoRw &rw) {
        for(auto it = neurons.begin(); it != neurons.end(); ++it) {
            (*it)->loadModel(rw);
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
            vector<Synapse*> added_synapses;
            for(size_t nj=0; nj<l_post.N; nj++) {
                if( (neurons[ni]->id != l_post[nj]->id) && (!l_post[nj]->hasConnection(neurons[ni]->id)) ) {
                    double prob = getUnif();
                    if( conf.prob > prob ) {
                        double dendrite_delay = sampleDelay(conf.dendrite_delay_gain, conf.dendrite_delay_rate);
                        Synapse *s = Factory::inst().createSynapse(conf.type, c, neurons[ni]->id, 0, dendrite_delay);
                        l_post[nj]->addSynapse(s);
                        added_synapses.push_back(s);
                    }
                }
            }
            for(auto it=added_synapses.begin(); it != added_synapses.end(); ++it) {
                Synapse *s = *it;
                s->w = conf.weight_per_neuron/added_synapses.size();
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
    double p_wta;
    bool wta;
    vector< Neuron *> neurons;
private:
    const NeuronConf *neuron_conf;
    const RuntimeGlobals *glob_c;
};
