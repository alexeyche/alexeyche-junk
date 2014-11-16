#pragma once

#include "neuron.h"

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
    Layer(size_t _id, size_t _size, const ConstObj *_c, const ActFunc *_act, const LearningRule *_lrule, const TuningCurve *_tc) { 
        init(_id,_size,_c,_act, _lrule, _tc);
    }

    virtual Neuron* addNeuron(const ConstObj *_c, const ActFunc *_act, const LearningRule *_lrule, const TuningCurve *_tc) {
        return new Neuron(global_neuron_index++, _c, _act, _lrule, _tc);
    }
    virtual void init(size_t _id, size_t _size, const ConstObj *_c, const ActFunc *_act, const LearningRule *_lrule, const TuningCurve *_tc) {
        id = _id;
        N = _size;
        for(size_t i=0; i<N; i++) {
            neurons.push_back( shared_ptr<Neuron>(addNeuron(_c, _act, _lrule, _tc)) );
        }
    }

    size_t size() {
        return neurons.size();
    }
    Neuron *operator[](size_t i) {
        return neurons[i].get();
    }
    void connect(Layer &l_post, const ConnectionConf &conf, const Constants &c) {
        for(size_t ni=0; ni<neurons.size(); ni++) {
            for(size_t nj=0; nj<l_post.N; nj++) {
                if(neurons[ni]->id != l_post[nj]->id) {
                    double prob = getUnif();
                    if( conf.prob > prob ) {
                        //cout << "conf.prob > prob " << conf.prob << " > " << prob << "\n";
                        Synapse* s = Factory::inst()->createSynapse(conf.type, c[conf.type], neurons[ni]->id, conf.weight);
                        //cout << "Adding synapse " << *s << " to neuron " << (*l_post)[nj]->id << "\n";
                        l_post[nj]->addSynapse(*s);
                        delete s;
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

//protected:    
    size_t id;
    size_t N;
    vector< shared_ptr<Neuron> > neurons;
};

