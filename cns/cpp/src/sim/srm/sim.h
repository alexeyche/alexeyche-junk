#ifndef SIM_H
#define SIM_H

#include <sim/socket/sim_socket_core.h>

#include "srm.h"


namespace srm {
    
    class TStatGrabber {
    public: 
        enum TStatType { None, Spike, Prob };
        
        typedef std::vector<std::vector<double> > TStat;
        
        std::map<const unsigned int, TStatType> ids;
        TStat s;
    };

    class Sim {
    public:        
        typedef TStatGrabber::TStatType TStatType;

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
                    double pi = stoch_elem[ni]->p(t(ti));
                    if(pi*dt > unif(ti, ni)) {
                        TTime &yn = stoch_elem[ni]->y;
                        Log::Info << "spike of " << ni << " at " << t(ti) << "\n";
                        // spike!
                        yn.push_back(t(ti));
                    }
                    if(t[ti] % 1 == 0) {
                        
                    }
                }
            }
            send_arma_mat(stoch_stat, "stoch_stat");
        }

        void addNeuron(Neuron *n, TStatType gt = TStatType.None) {
            if(dynamic_cast<StochasticNeuron*>(n)) {
                stoch_elem.push_back(dynamic_cast<StochasticNeuron*>(n));            
            }
            if(gt != TStatType.None) {
                sg.addStatProvider(n);
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
        
        TStatGrabber sg;
    };

}


#endif
