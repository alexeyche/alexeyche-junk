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
    
    bool check_relation(const unsigned int &id1, const unsigned int &id2, std::map<int, std::set<int> > &uniq_rels) {            
        if(uniq_rels.find(id1) != uniq_rels.end()) {
            if(uniq_rels[id1].find(id2) != uniq_rels[id1].end()) {
                return true;
            }
        } else {
            uniq_rels[id1] = std::set<int>();
        }
        return false;
    }        
    
    void connect(NeuronGroup *gr_left, NeuronGroup *gr_right, TConnType::EConnType ct, double start_weight) {
        std::map<int, std::set<int> > uniq_rels;
            
        for(size_t gli=0; gli<gr_left->size(); gli++) {
            for(size_t gri=0; gri<gr_right->size(); gri++) {
                const unsigned int &idl = gr_left->at(gli)->id();
                const unsigned int &idr = gr_right->at(gri)->id();
                
                if(idr == idl) { continue; } // Say No to self-connection
                
                if((ct == TConnType::FeedForward)||(ct == TConnType::AllToAll)) {
                    if(!check_relation(idl, idr, uniq_rels)) {
                        //Log::Info << idl << "->" << idr << "\n";
                        gr_right->at(gri)->add_input( gr_left->at(gli) , start_weight);
                        uniq_rels[idl].insert(idr);
                    }                        
                }
                if(ct == TConnType::AllToAll) {
                    if(!check_relation(idr, idl, uniq_rels)) {
                        //Log::Info << idr << "->" << idl << "\n";
                        gr_left->at(gli)->add_input( gr_right->at(gri) , start_weight);
                        uniq_rels[idr].insert(idl);
                    }
                }       
            }
        }

    }

};


#endif
