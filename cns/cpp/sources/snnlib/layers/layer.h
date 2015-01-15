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
        init(_size, _wta, nc, c, run_glob_c, true);
    }

    virtual void init(size_t _size, bool _wta, const NeuronConf &nc, const Constants &c, RuntimeGlobals *run_glob_c, bool learning) {
        id = global_layer_index++;
        N = _size;
        neuron_conf = &nc;
        glob_c = run_glob_c;
        p_wta = 0.0;
        wta = _wta;
        for(size_t ni=0; ni<N; ni++) {
            double axon_delay = nc.axon_delay_distr->getSample();

            Neuron *n = Factory::inst().createNeuron(nc.neuron, ni, c, run_glob_c, axon_delay);
            ActFunc *act_f = Factory::inst().createActFunc(nc.act_func, c, n);
            n->setActFunc(act_f);

            if(!nc.reward_modulation.empty()) {
                n->setRewardModulation(Factory::inst().createRewardModulation(nc.reward_modulation, c, n, run_glob_c));
            }

            WeightNormalization *wnorm = nullptr;
            if(!nc.weight_normalization.empty()) {
                wnorm = Factory::inst().createWeightNormalization(nc.weight_normalization, c, n);
            }

            if( (!nc.learning_rule.empty())&&(learning) ) {
                n->setLearningRule(Factory::inst().createLearningRule(nc.learning_rule, c, n, act_f, wnorm));
            }
            if(!nc.tuning_curve.empty()) {
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
            for(size_t nj=0; nj<l_post.N; nj++) {
                if ((neurons[ni]->id != l_post[nj]->id) && (!l_post[nj]->hasConnection(neurons[ni]->id)) ) {
                    double prob = getUnif();
                    if( conf.prob > prob ) {
                        double dendrite_delay = conf.dendrite_delay_distr->getSample();
                        double weight = conf.weight_distr->getSample();
                        Synapse *s = Factory::inst().createSynapse(conf.type, c, neurons[ni]->id, weight, dendrite_delay);
                        l_post[nj]->addSynapse(s);
                    }
                }
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
