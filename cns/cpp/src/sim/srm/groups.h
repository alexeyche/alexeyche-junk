#ifndef GROUPS_H
#define GROUPS_H

#include "neurons.h"
#include <limits>

namespace srm {
    class NeuronGroup : public SimElement { 
    public:
        NeuronGroup(bool preCalculate) : SimElement(preCalculate) {}
        Neuron* operator[](size_t ind) {
            return group[ind];
        }
        Neuron* at(size_t ind) {
            return group[ind];
        }
        size_t size() {
            return group.size();
        }
        
        std::vector<Neuron*> group;
    };

    class TimeSeriesGroup : public NeuronGroup {
    public:    
        struct TPattern {
            TPattern(size_t group_n, size_t n, double prob, double dur) : pattern(group_n, n, fill::zeros), pattProb(prob), 
                                                                          pattDur(dur), dt(pattDur/n), dedicatedTime(-1,-1) { }
            mat pattern;
            double pattProb;
            double pattDur;
            double dt;
            std::pair<double, double> dedicatedTime;
        };
        TimeSeriesGroup(size_t n, double refr, double pattProb) : NeuronGroup(true), refrTime(refr), pattProb(pattProb) { 
            for(size_t gi=0; gi<n; gi++) {
                group.push_back(new DetermenisticNeuron());
            }
        }
        ~TimeSeriesGroup() {
            for(size_t gi=0; gi<group.size(); gi++) {
                delete group[gi];
            }
            group.clear();
        }

        void loadPatternFromFile(std::string csv_file, double pattDur, double pattProb) {
            mat ts;
            data::Load(csv_file, ts, true, false); 
            if(ts.n_cols>1) { Log::Warn << "More than one column was found. Ignoring others\n"; }
            TPattern p(group.size(), ts.n_rows, pattProb, pattDur);
            size_t ci=0; // first column

            double patt_dt = 0;
            double lb = min(ts.col(ci));
            double hb = max(ts.col(ci));
            double dt = (hb-lb)/(group.size()-1);
            for(size_t ri=0; ri<ts.n_rows; ri++) {
                double val = ts(ri, ci);                
                int ind_n = (val-lb)/dt;
                patt_dt += pattDur/ts.n_rows;
                p.pattern(ind_n, ri) = patt_dt;
            }
            patterns.push_back(p);
        }
        void preCalculate(double T0, double Tmax, double dt) {
            Log::Info << "Precalculating TimeSeriesGroup\n"; 
            for(size_t ni=0; ni<group.size(); ni++) {
                group[ni]->y.clean();
            }
            vec mean_rate(group.size(), fill::zeros);
            for(size_t pi=0; pi<patterns.size(); pi++) {
                mat &pattern = patterns[pi].pattern;
                mean_rate += mean(pattern, 1)/patterns.size();
            }
            //meta_rate = mean_rate/10;
            vec t = linspace<vec>(T0, Tmax, (int)Tmax/dt);
            vec unif(t.n_elem, fill::randu);
            double refrTime_cur = 0;
            int patt_id=-1; // pattern that we choosed
            double patt_ti=0;  // current pattern time id
            int last_patt_id = -1;
            for(size_t ti=0; ti<t.n_elem; ti++) {
                if (refrTime_cur > 0) { 
                    refrTime_cur -= dt;
                } else if(patt_id<0) { // choosing pattern
                    if(last_patt_id>=patterns.size()-1) {
                        last_patt_id = -1;
                    }
                    if(last_patt_id <0) { 
                        patt_id = 0; 
                    }
                    else { 
                        patt_id = last_patt_id +1;
                    }
                    Log::Info << "What time is it? Pattern time! ("  << t(ti) << ")\n";
                    Log::Info << "(" << t(ti) << ") pattern# " << patt_id << "\n";
                    patterns[patt_id].dedicatedTime.first = t(ti);
                    //if(pattProb*dt>unif(ti)) {
                    //    Log::Info << "What time is it? Pattern time! ("  << t(ti) << ")\n";
                    //    double cump=0;
                    //    vec unif_patt(patterns.size(), fill::randu);
                    //    for(size_t pi=0; pi<patterns.size(); pi++) {
                    //        cump += patterns[pi].pattProb;
                    //        if(cump>unif_patt(pi)) {
                    //            patt_id = pi;
                    //            Log::Info << "(" << t(ti) << ") pattern# " << patt_id << " cump: " << cump << "\n";
                    //            break;
                    //        }
                    //    }
                    //}                        
                } 
                if(patt_id<0) {
                //    // filling with poisson process
                //    vec unif_poiss(group.size(), fill::randu);                    
                //    uvec bl = (mean_rate*dt>unif_poiss);
                //    uvec fired = find(bl == 1);
                //    for(size_t fi=0; fi<fired.n_elem; fi++) {
                //        group[fired(fi)]->y.push_back(t(ti));
                //    }
                } else {
                    mat &pattern = patterns[patt_id].pattern;
                    if(patt_ti >= patterns[patt_id].pattDur) {  // pattern ended
                        patterns[patt_id].dedicatedTime.second = t(ti);
                        last_patt_id = patt_id;
                        patt_id = -1;
                        patt_ti = 0;
                        refrTime_cur = refrTime;
                        continue;
                    }
                    //Log::Info << "pattern.ncol " << pattern.n_cols << " pattern.nrow " << pattern.n_rows << "\n";
                    //Log::Info << "patt_id " << patt_id << " patt_ti " << patt_ti << " patt.dt " << patterns[patt_id].dt << "\n";    
                    double patt_index = patt_ti/patterns[patt_id].dt;
                    int patt_index_int = floor(patt_index+0.001); 
                    //Log::Info << "patt_index " <<  patt_index << "\n";
                    //Log::Info << "patt_index_int " <<  patt_index_int << "\n";
                    if( patt_index - patt_index_int < 1e-10) {  //machine precision stuff
                        uvec fired = find(pattern.col(patt_index_int)>0);
                        for(size_t fi=0; fi<fired.n_elem; fi++) {
                            group[fired(fi)]->y.push_back(t(ti));
                        }
                    }
                    patt_ti+=dt;
                }                    
            }
        }
        std::vector<TPattern> patterns;
        double refrTime;
        double pattProb;
    };


    class SrmNeuronGroup : public NeuronGroup {
    public:
        SrmNeuronGroup(size_t num) : NeuronGroup(false) {
            for(size_t ni=0; ni<num; ni++) {
                group.push_back(new SrmNeuron());
            }
        }
        ~SrmNeuronGroup() {  
            for(size_t ni=0; ni<group.size(); ni++) { 
                delete group[ni];
            }
        }
    };

}

#endif
