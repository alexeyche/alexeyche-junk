#ifndef SIM_H
#define SIM_H

#include <sim/socket/sim_socket_core.h>

#include "srm.h"


namespace srm {
    
    class TStatGrabber {
    public: 
        enum TStatType { None, Spike, Prob };
        typedef std::pair<std::vector<double>*, TStatType> TStatBox;
        typedef std::map<const unsigned int, TStatBox> TStat;
        
        ~TStatGrabber() {
            for(TStat::iterator it = ids.begin(); it != ids.end(); it++) {
                delete it->second.first;
            }
        }

        void addStatProvider(StochasticNeuron *n, TStatType st) {
            TStatBox b;
            b.first = new std::vector<double>();
            b.second = st;
            ids[n->id()] = b;
        }
        
        void addStat(StochasticNeuron *n, double s) {
            TStat::iterator it = ids.find(n->id());
            it->second.first->push_back(s);
        }
        
        TStatType getStatType(StochasticNeuron *n) {
            TStat::iterator it = ids.find(n->id());
            if(it != ids.end()) {
                return it->second.second;
            } else {
                TStatType s = None;
                return s;
            }
        }
        void sendStat() {
            for(TStat::iterator it = ids.begin(); it != ids.end(); it++) {
                vec s(*(it->second.first));            
                send_arma_mat(s, "neuron", &it->first); 
            }
        }
        TStat ids;
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
                    TStatType nti = sg.getStatType(stoch_elem[ni]);
                    
                    if(pi*dt > unif(ti, ni)) {
                        TTime &yn = stoch_elem[ni]->y;
                        Log::Info << "spike of " << ni << " at " << t(ti) << "\n";
                        // spike!
                        yn.push_back(t(ti));
                        if(nti == TStatGrabber::Spike) {
                            sg.addStat(stoch_elem[ni], t(ti));
                        }
                    }
                   if(nti == TStatGrabber::Prob) {
                        sg.addStat(stoch_elem[ni], pi);
                   }
                }
            }
            sg.sendStat();
        }

        void addNeuron(Neuron *n, TStatType gt = TStatGrabber::None) {
            if(dynamic_cast<StochasticNeuron*>(n)) {
                stoch_elem.push_back(dynamic_cast<StochasticNeuron*>(n));            
                if(gt != TStatGrabber::None) {
                    sg.addStatProvider(stoch_elem.back(), gt);
                }               
            }
            
        }
        void addRecNeuron(Neuron *n, TStatType gt = TStatGrabber::None) {
            std::set<unsigned int> hist;
            addRecNeuronFn(n, gt, hist);
        }

        std::vector<StochasticNeuron*> stoch_elem;
    private:        
        void addRecNeuronFn(Neuron *n, TStatType gt, std::set<unsigned int> &hist) {
            addNeuron(n, gt);
            hist.insert(n->id());
            for(size_t ni=0; ni<n->in.size(); ni++) {
                if(hist.find(n->in[ni]->id()) == hist.end()) { 
                    addRecNeuronFn(n->in[ni], gt, hist);
                }
            }
        }
        
        TStatGrabber sg;
    };

}


#endif
