#ifndef SIM_H
#define SIM_H

#include <sim/socket/sim_socket_core.h>
#include <unistd.h>

#include "neurons.h"
#include "groups.h"
#include "entropy_grad.h"

namespace srm {
    const double sec = 1000; //ms
    const double ms = sec/1000; // ms
    

    class NeuronGroupStat {
    public:        
        NeuronGroupStat(NeuronGroup *g, int stat_length) : stat(g->size(), stat_length) { 
            for(size_t ni=0; ni<g->size(); ni++) {
                ids.insert(g->at(ni)->id());
            }
        }
        void writeStat(Neuron *n, double stat_value, size_t stat_index) {
            std::set<int>::const_iterator it = ids.find(n->id());
            if(it != ids.end()) {
                size_t ind = std::distance(ids.begin(), it); 
                stat(ind, stat_index) = stat_value; 
            }
        }

        mat stat;
        std::set<int> ids;
    };
   
    class TStatListener {
    public: 
        std::vector<StochasticNeuron*> neuron_to_listen;
        std::vector<SrmNeuronGroup*> group_to_listen;
    };
    
    
    class TRunType {
    public:
        enum ERunType { Run, RunAndLearn };
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
        void run(double Tdur, double dt, TRunType::ERunType rt, bool verbose=true, bool send_data=true) {
            double Tmax = T0 + Tdur;
            if(verbose) {
                Log::Info << "Finding elements to precalculate\n";
            }                
            for(size_t el_i=0; el_i<sim_elem.size(); el_i++) { 
                if(sim_elem[el_i]->isNeedPreCalc()) {
                    sim_elem[el_i]->preCalculate(T0, Tmax, dt);
//                    TimeSeriesGroup *gr = (TimeSeriesGroup*)sim_elem[el_i];
//                    for(unsigned int i=0; i<gr->group.size(); i++) {
//                        Log::Info << "n " << i << ":\n";
//                        Log::Info << gr->group[i]->y.size() << "\n";
//                        gr->group[i]->y.print();
//                        //usleep(100000);
//                    }
//                    send_arma_mat(gr->group[9]->y, "d_stat", 9);
                }
            }
            if(verbose) { Log::Info << "Done\n"; }
            vec t = linspace<vec>(T0, Tmax, (int)Tmax/dt);
           
            std::vector<NeuronGroupStat> stats;
            std::map<unsigned int, NeuronGroupStat*> stat_map;
            for(size_t gi=0; gi< sg.group_to_listen.size(); gi++) {
                stats.push_back( NeuronGroupStat(sg.group_to_listen[gi], t.n_elem) );
                for(size_t ni=0; ni < sg.group_to_listen[gi]->size(); ni++) {
                    stat_map[sg.group_to_listen[gi]->at(ni)->id()] = &stats.back();
                }
            }
            int max_spikes = 0;
            double learn_dt = 5;
            double learn_dti = 0;
            std::vector<SrmNeuron*> neuron_fired; // in window

            mat unif(t.n_elem, stoch_elem.size(), fill::randu);
            for(size_t ti=0; ti<t.n_elem; ti++) {
                for(size_t ni=0; ni<stoch_elem.size(); ni++) {
                    double pi = stoch_elem[ni]->p(t(ti));
                    
                    std::map<unsigned int, NeuronGroupStat*>::iterator st_it = stat_map.find(stoch_elem[ni]->id());
                    if(st_it != stat_map.end()) {
                        st_it->second->writeStat(stoch_elem[ni], pi, ti);
                    }                          
                    
                    if(pi*dt > unif(ti, ni)) {
                        TTime &yn = stoch_elem[ni]->y;
                        if(verbose) { Log::Info << "spike of " << ni << " at " << t(ti) << "\n"; }
                        // spike!
                        yn.push_back(t(ti));
                        if(yn.size() > max_spikes) {
                            max_spikes = yn.size();
                        }
                        if(SrmNeuron *n = dynamic_cast<SrmNeuron*>(stoch_elem[ni])) {
                            neuron_fired.push_back(n);
                        }                            
                    }
                                            
                }
                if(rt == TRunType::RunAndLearn) {
                    if(learn_dti>=learn_dt) {
                        for(size_t ni=0; ni<neuron_fired.size(); ni++) { 
                            TEntropyGrad eg(neuron_fired[ni], t(ti)-learn_dt, t(ti));
                            vec dHdw = eg.grad_1eval();
                            for(size_t wi=0; wi<dHdw.n_elem; wi++) { Log::Info << dHdw(wi) << ", "; neuron_fired[ni]->w[wi] -= 0.01 * dHdw(wi);  }
                            Log::Info << "\n";                   
                        }                    
                        neuron_fired.clear();
                        learn_dti = 0;
                    } else {                        
                        learn_dti+=dt;
                    }               
                }                    
            }
            if(verbose) { Log::Info << "Sending statistics to 7778\n"; }
            for(size_t i=0; i<stats.size(); i++) {
                send_arma_mat(stats[i].stat, "gr_stat", i);
            }
            if (send_data) {
                mat raster(stoch_elem.size(), max_spikes, fill::zeros);
                for(size_t ni=0; ni<stoch_elem.size(); ni++) {
                    for(size_t yi=0; yi<stoch_elem[ni]->y.size(); yi++) {
                        raster(ni, yi) = stoch_elem[ni]->y[yi];                
                    }
                }
                send_arma_mat(raster, "raster");
            }                
            if(verbose) { Log::Info << "Done\n"; }
            //T0 = Tmax;
        }

        void addNeuron(Neuron *n) {
            if(dynamic_cast<StochasticNeuron*>(n)) {
                stoch_elem.push_back(dynamic_cast<StochasticNeuron*>(n));            
            }
        }

        void addNeuronGroup(NeuronGroup *gr) {
            if(SimElement *se = dynamic_cast<SimElement*>(gr)) {
                sim_elem.push_back(se);
            }
            if(SrmNeuronGroup *srg = dynamic_cast<SrmNeuronGroup*>(gr)) {
                for(size_t ni=0; ni< srg->size(); ni++) {
                    stoch_elem.push_back(dynamic_cast<StochasticNeuron*>(srg->at(ni)) );
                }
            }
        }
        void addRecNeuron(Neuron *n) {
            std::set<unsigned int> hist;
            addRecNeuronFn(n, hist);
        }
    
        void addStatListener(StochasticNeuron *n) {
            sg.neuron_to_listen.push_back(n);
        }
        
        void addStatListener(SrmNeuronGroup *n) {
            sg.group_to_listen.push_back(n);    
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
