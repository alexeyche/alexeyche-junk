#ifndef CONNECTIONS_H
#define CONNECTIONS_H


#include "groups.h"


namespace srm {
    void connectFeedForward(NeuronGroup *gr, Neuron *n, vec weights) {
        if(gr->group.size() != weights.n_elem) {
            throw SrmException("Can't connect: Weights vector size do no equal to NeuronGroup size\n");
        }
        for(size_t gi=0; gi<gr->group.size(); gi++) {
            n->add_input(gr->group[gi], weights(gi));
        }
    }
    void connectFeedForward(NeuronGroup *gr, Neuron *n, double start_weight) {
        vec w(gr->group.size());
        w.fill(start_weight);
        connectFeedForward(gr, n, w);
    }
    class TConnType {
    public:    
        enum EConnType { FeedForward, AllToAll };
    };
    void connect(NeuronGroup *gr_left, NeuronGroup *gr_right, TConnType::EConnType ct, double start_weight) {
        for(size_t gli=0; gli<gr_left->size(); gli++) {
            for(size_t gri=0; gri<gr_right->size(); gri++) {
                if(gr_right->group[gri]->id() == gr_left->group[gli]->id()) { continue; }
                if((ct == TConnType::FeedForward)||(ct == TConnType::AllToAll)) {
                    Log::Info << gr_left->group[gli]->id() << "->" << gr_right->group[gri]->id() << "\n";
                    gr_right->group[gri]->add_input( gr_left->group[gli] , start_weight);
                }
                if(ct == TConnType::AllToAll) {
                    Log::Info << gr_right->group[gri]->id() << "->" << gr_left->group[gli]->id() << "\n";
                    gr_left->group[gli]->add_input( gr_right->group[gri] , start_weight);
                }
            }                    
        }
    }

};


#endif
