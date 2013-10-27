#ifndef SIM_H
#define SIM_H

#include "srm.h"

namespace srm {
    class Sim {
    public:        
        Sim() {}
        ~Sim() { 
            for(size_t ni=0; ni<stoch_elem.size(); ni++) { 
                delete stoch_elem[ni]; 
            }
            stoch_elem.clear();
            Log::Info << "Cleaning Sim\n"; 
        }            
        void run(double Tmax, double dt=0.1) {
            vec t = linspace<vec>(0.0, Tmax, (int)Tmax/dt);
            mat unif(t.n_elem, stoch_elem.size(), fill::randu);
            for(size_t ti=0; ti<t.n_elem; ti++) {
                for(size_t ni=0; ni<stoch_elem.size(); ni++) {
                    Log::Info << "p: " << stoch_elem[ni]->p(t[ti]) << " ";
                    if(stoch_elem[ni]->p(t[ti])*dt > unif[ti, ni]) {
                        stoch_elem[ni]->y << t[ti] << endr;
                    }
                }
            }
        }

        void addNeuron(Neuron *n) {
            if(dynamic_cast<StochasticNeuron*>(n)) {
                stoch_elem.push_back(dynamic_cast<StochasticNeuron*>(n));            
            }
        }
        void addRecNeuron(Neuron *n) {
            std::set<unsigned int> hist;
            addRecNeuronFn(n, hist);
        }

        std::vector<StochasticNeuron*> stoch_elem;
    private:        
        void addRecNeuronFn(Neuron *n, std::set<unsigned int> &hist) {
            addNeuron(n);
            hist.insert(n->id());
            for(size_t ni=0; ni<n->in.size(); ni++) {
                if(hist.find(n->in[ni]->id()) == hist.end()) { 
                    addRecNeuronFn(n->in[ni], hist);
                }
            }
        }
    };

}


#endif
