#ifndef SIM_H
#define SIM_H

#include <sim/socket/sim_socket_core.h>
#include <unistd.h>

#include "neurons.h"
#include "groups.h"

namespace srm {
    const double sec = 1000; //ms
    const double ms = sec/1000; // ms
    

    class TStatListener {
    public: 
        enum TStatType { None, Spike, Prob, Pot };
        typedef std::map<TStatType, std::vector<double>* > TStatBox;
        typedef std::map<const unsigned int, TStatBox> TStat;
        
        ~TStatListener() {
            for(TStat::iterator it = ids.begin(); it != ids.end(); it++) {
                for(TStatBox::iterator it_s = it->second.begin(); it_s != it->second.end(); it_s++) {
                    delete it->second[it_s->first];
                }
            }
        }

        void addStatProvider(StochasticNeuron *n, TStatType st) {
            TStat::iterator it = ids.find(n->id());
            if(it == ids.end()) {
                TStatBox b;
                ids[n->id()] = b;
                it = ids.find(n->id());
            } 
            TStatBox::iterator it_b = it->second.find(st);
            if(it_b == it->second.end()) {
                it->second[st] = new std::vector<double>();
            } else {
                Log::Info << "We already have this type of TStatType: " << st << "\n";
            }
        }
        
        void addStat(StochasticNeuron *n, TStatType st, double s) {
            TStat::iterator it = ids.find(n->id());
            if(it != ids.end()) {
                TStatBox::iterator it_s = it->second.find(st);
                if(it_s != it->second.end()) {
                    it_s->second->push_back(s);
                }
            }
        }
        
        void sendStat() {
            for(TStat::iterator it = ids.begin(); it != ids.end(); it++) {
                for(TStatBox::iterator it_s = it->second.begin(); it_s != it->second.end(); it_s++) {
                    vec s(*(it_s->second));
                    std::string stat_name;
                    if(it_s->first == Spike) {
                        stat_name = "n_spike";
                    }
                    if(it_s->first == Prob) {
                        stat_name = "n_prob";
                    }
                    if(it_s->first == Pot) {
                        stat_name = "n_pot";
                    }
                    send_arma_mat(s, stat_name, &it->first); 
                }
            }
        }
        TStat ids;
    };

    class Sim {
    public:        
        Sim() : T0(0.0) {}
        ~Sim() { 
//            for(size_t ni=0; ni<stoch_elem.size(); ni++) { 
//                delete stoch_elem[ni]; 
//            }
//            stoch_elem.clear();
            Log::Info << "Cleaning Sim\n"; 
        }            
        void run(double Tdur, double dt) {
            double Tmax = T0 + Tdur;
            Log::Info << "Finding elements to precalculate\n";
            for(size_t el_i=0; el_i<sim_elem.size(); el_i++) { 
                if(sim_elem[el_i]->isNeedPreCalc()) {
                    sim_elem[el_i]->preCalculate(T0, Tmax, dt);
//                    TimeSeriesGroup *gr = (TimeSeriesGroup*)sim_elem[el_i];
//                    for(unsigned int i=0; i<gr->group.size(); i++) {
//                        Log::Info << "n " << i << ":\n";
//                        gr->group[i]->y.print();
//                        //send_arma_mat(gr->group[i]->y, "d_stat", &i);
//                        //usleep(100000);
//                    }
                }
            }
            Log::Info << "Done\n"; 
            vec t = linspace<vec>(T0, Tmax, (int)Tmax/dt);
                            
            mat unif(t.n_elem, stoch_elem.size(), fill::randu);
            for(size_t ti=0; ti<t.n_elem; ti++) {
                for(size_t ni=0; ni<stoch_elem.size(); ni++) {
                    double pi = stoch_elem[ni]->p(t(ti));
                    sg.addStat(stoch_elem[ni], TStatListener::Pot, stoch_elem[ni]->u(t(ti)));
                    sg.addStat(stoch_elem[ni], TStatListener::Prob, pi);
                    if(pi*dt > unif(ti, ni)) {
                        TTime &yn = stoch_elem[ni]->y;
                        Log::Info << "spike of " << ni << " at " << t(ti) << "\n";
                        // spike!
                        yn.push_back(t(ti));
                        sg.addStat(stoch_elem[ni], TStatListener::Spike, t(ti));
                    }
                }
            }
            Log::Info << "Sending statistics to 7778\n";
            sg.sendStat();
            Log::Info << "Done\n";
            T0 = Tmax;
        }

        void addNeuron(Neuron *n) {
            if(dynamic_cast<StochasticNeuron*>(n)) {
                stoch_elem.push_back(dynamic_cast<StochasticNeuron*>(n));            
            }
                
        }

        void addNeuronGroup(NeuronGroup *gr) {
            if(dynamic_cast<SimElement*>(gr)) {
                sim_elem.push_back(dynamic_cast<SimElement*>(gr));
            }
        }
        void addRecNeuron(Neuron *n) {
            std::set<unsigned int> hist;
            addRecNeuronFn(n, hist);
        }
    
        void addStatListener(StochasticNeuron *n, TStatListener::TStatType st) {
            sg.addStatProvider(n, st);
        }

        std::vector<StochasticNeuron*> stoch_elem;
        std::vector<SimElement*> sim_elem;
        
        double T0;
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
        
        TStatListener sg;
    };

}


#endif
