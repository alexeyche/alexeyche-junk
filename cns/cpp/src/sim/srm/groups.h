#ifndef GROUPS_H
#define GROUPS_H

#include "neurons.h"

namespace srm {
    class NeuronGroup : public SimElement { 
    public:
        NeuronGroup(bool preCalculate) : SimElement(preCalculate) {}

        std::vector<Neuron*> group;
    };

    class TimeSeriesGroup : public NeuronGroup {
    public:    
        struct TPattern {
            TPattern(size_t group_n, size_t n, double prob, double dur) : pattern(group_n, n, fill::zeros), pattProb(prob), pattDur(dur) {}
            mat pattern;
            double pattProb;
            double pattDur;
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
        void preCalculate(double Tmax, double dt) {
            Log::Info << "Precalculating TimeSeriesGroup\n"; 
            vec mean_rate(group.size(), fill::zeros);
            for(size_t pi=0; pi<patterns.size(); pi++) {
                mat &pattern = patterns[pi].pattern;
                mean_rate += mean(pattern, 1)/patterns.size();
            }
            //meta_rate = mean_rate/10;
            vec t = linspace<vec>(0, Tmax, (int)Tmax/dt);
            vec unif(t.n_elem, fill::randu);
            double refrTime_cur = 0;
            int patt_id=-1; // pattern that we choosed
            size_t patt_ti=0;  // current pattern time id
            for(size_t ti=0; ti<t.n_elem; ti++) {
                if (refrTime_cur > 0) { 
                    refrTime_cur -= dt;
                } else if(patt_id<0) { // choosing pattern
                    if(pattProb*dt>unif(ti)) {
//                        Log::Info << "What time is it? Pattern time! ("  << t(ti) << ")\n";
                        double cump=0;
                        vec unif_patt(patterns.size(), fill::randu);
                        for(size_t pi=0; pi<patterns.size(); pi++) {
                            cump += patterns[pi].pattProb;
                            if(cump>unif_patt(pi)) {
                                patt_id = pi;
                                Log::Info << "(" << t(ti) << ") pattern# " << patt_id << " cump: " << cump << "\n";
                                break;
                            }
                        }
                    }                        
                } 
                if(patt_id<0) {
                    // filling with poisson process
                    vec unif_poiss(group.size(), fill::randu);                    
                    uvec bl = (mean_rate*dt>unif_poiss);
                    uvec fired = find(bl == 1);
                    for(size_t fi=0; fi<fired.n_elem; fi++) {
                        group[fired(fi)]->y.push_back(t(ti));
                    }
                } else {
                    mat &pattern = patterns[patt_id].pattern;
                    if(patt_ti >= pattern.n_cols) {  // pattern ended
                        patt_id = -1;
                        patt_ti = 0;
                        refrTime_cur = refrTime;
                        continue;
                    }
                    uvec fired = find(pattern.col(patt_ti)>0);
                    for(size_t fi=0; fi<fired.n_elem; fi++) {
                        group[fired(fi)]->y.push_back(t(ti));
                    }
                    patt_ti++;
                }                    
            }
        }
        std::vector<TPattern> patterns;
        double refrTime;
        double pattProb;
    };



}

#endif
