#ifndef SIM_H
#define SIM_H

#include <sim/socket/sim_socket_core.h>
#include <unistd.h>

#include "neurons.h"
#include "groups.h"

#include "llh.h"

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
            std::set<int>::iterator it = ids.find(n->id());
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
        enum ERunType { Run, RunAndLearnEntropy, RunAndLearnSTDP, RunAndLearnLogLikelyhood, RunAndLearnStabilize };
    };

    class Sim {
    public:        
        
        Sim(double rate=0.1) : T0(0.0), learning_rate(rate) {}
        ~Sim() { 
//            for(size_t ni=0; ni<stoch_elem.size(); ni++) { 
//                delete stoch_elem[ni]; 
//            }
//            stoch_elem.clear();
            Log::Info << "Cleaning Sim\n"; 
        }            

        typedef std::pair<NeuronGroupStat*,NeuronGroupStat*> pair_stat;

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
//                        Log::Info << "size of y: " << gr->group[i]->y.size() << "; y:\n";
//                        gr->group[i]->y.print();
//                    }
//                    send_arma_mat(gr->group[9]->y, "d_stat", 9);
                }
            }
            if(rt == TRunType::RunAndLearnSTDP) {
                for(size_t ni=0; ni<stoch_elem.size(); ni++) {
                    SrmNeuron *n = dynamic_cast<SrmNeuron*>(stoch_elem[ni]);
                    if(n) {
                        n->stdp_learning = true;
                    }
                }
            }
            if(verbose) { Log::Info << "Done\n"; }
            vec t = linspace<vec>(T0, Tmax, (int)Tmax/dt);
           
            std::vector<NeuronGroupStat*> probstats;
            std::vector<NeuronGroupStat*> potstats;

            std::map<unsigned int, pair_stat> stat_map;
            for(size_t gi=0; gi< sg.group_to_listen.size(); gi++) {
                probstats.push_back(new NeuronGroupStat(sg.group_to_listen[gi], t.n_elem) );
                potstats.push_back(new NeuronGroupStat(sg.group_to_listen[gi], t.n_elem) );
                for(size_t ni=0; ni < sg.group_to_listen[gi]->size(); ni++) {
                    stat_map[sg.group_to_listen[gi]->at(ni)->id()] = pair_stat(probstats.back(), potstats.back());
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
                    
                    std::map<unsigned int, pair_stat>::iterator st_it = stat_map.find(stoch_elem[ni]->id());
                    if(st_it != stat_map.end()) {
                        st_it->second.first->writeStat(stoch_elem[ni], pi, ti);
                        st_it->second.second->writeStat(stoch_elem[ni], stoch_elem[ni]->u(t(ti)),ti);
                    }                          
                    if(pi*dt > unif(ti, ni)) {
                        TTime &yn = stoch_elem[ni]->y;
                    #if VERBOSE >= 1
                        printf("spike of %d at %f (p == %f > U(x) == %f) \n", stoch_elem[ni]->id(), t(ti), pi*dt, unif(ti, ni));
                    #endif
                        yn.push_back(t(ti));
                        if(yn.size() > max_spikes) {
                            max_spikes = yn.size();
                        }
                        
                        if(SrmNeuron *n = dynamic_cast<SrmNeuron*>(stoch_elem[ni])) {
                            neuron_fired.push_back(n);
                        }                                                   
                                                    
                    } 
                                       
                }
                if((rt == TRunType::RunAndLearnLogLikelyhood)||(rt == TRunType::RunAndLearnStabilize)) {
                    if(learn_dti>=learn_dt) {
                        for(size_t ni=0; ni<neuron_fired.size(); ni++) {
                            SrmNeuron *n = neuron_fired[ni];
                            //SrmNeuron *n = dynamic_cast<SrmNeuron*>(stoch_elem[ni]);
                            double yl = n->y.last();
                            //if(yl<t(ti)-2*learn_dt) {
                            //    yl = t(ti)-learn_dt;
                            //    Log::Info << "Spike wasn't found. Taking " << t(ti)-learn_dt << "\n";
                            //}
                            for(size_t it=0; it<100; it++) {
                                TLogLikelyhood llh(n, yl-learn_dt, yl+learn_dt);
                                vec dPdw = llh.grad();
                                printf("iter %zu  -- ", it);
                                for(size_t wi=0; wi<n->w.size(); wi++) {
                                    n->w[wi] += learning_rate * dPdw(wi);
                                #if VERBOSE >= 1                                
                                    if(dPdw(wi) != 0) 
                                        printf("syn:%zu = %f, ", wi, dPdw(wi));
                                #endif                                
                                }
                            #if VERBOSE >= 1
                                printf("\n");
                            #endif 
                            }
                        }
                        learn_dti = 0;
                        neuron_fired.clear();
                    } else {
                        learn_dti += dt;
                    }
                }
            }

            if(verbose) { Log::Info << "Sending statistics to 7778\n"; }
            
            if (send_data) {
                for(size_t i=0; i<probstats.size(); i+=1) {
                    send_arma_mat(probstats[i]->stat, "gr_stat_prob", i, true);
                }
                for(size_t i=0; i<potstats.size(); i+=1) {
                    send_arma_mat(potstats[i]->stat, "gr_stat_pot", i, true);
                }

            }                
            if(max_spikes !=0) {
                mat rasterc(stoch_elem.size(), max_spikes);
                rasterc.fill(-1);
                for(size_t ni=0; ni<stoch_elem.size(); ni++) {
                    for(size_t yi=0; yi<stoch_elem[ni]->y.size(); yi++) {
                        rasterc(ni, yi) = stoch_elem[ni]->y[yi];                
                    }
                }
                if(send_data) send_arma_mat(rasterc, "raster", time(NULL), true);
                raster = rasterc;
            }                    
            if(verbose) { Log::Info << "Done\n"; }
            //T0 = Tmax;
            for(size_t gi=0; gi< sg.group_to_listen.size(); gi++) {
                delete probstats[gi];
                delete potstats[gi];
            }
           
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
        mat raster;
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
        double learning_rate;
    };

}


#endif
