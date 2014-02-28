#ifndef NETSIM_H
#define NETSIM_H

#include <RcppArmadillo.h>

using namespace Rcpp;
using namespace std;

#include "sim.h"

class NetSim {
public:    
    NetSim(const List &net, const int T_size, const double &dt_, int num_neurons) : sp_spikes(net.size(), T_size+1), dt(dt_) {
        for(size_t sp_i=0; sp_i < net.size(); sp_i++) {
            NumericVector sp = net[sp_i];
            for(size_t spike_i=0; spike_i < sp.size(); spike_i++) {
                if((int)(sp[spike_i]/dt) > T_size) {
                    break;
                }
                sp_spikes(sp_i, (int)(sp[spike_i]/dt)) = 1;
            }
        }
//        sp_spikes.print();
    }
//    arma::vec getNumSpikesV(const arma::uvec &i, const double &t) {
//        arma::uvec c_id = i - 1;
//        std::cout << "SpMat read access: " << c_id << ":" << (int)(t/dt) << "\n";
//        arma::SpSubview<double> all_spikes = sp_spikes.col((int)(t/dt));
//        std::cout << "ok: " << all_spikes  << "\n";
//        arma::vec n_spikes = all_spikes(c_id);
//        return n_spikes;
//    }
    int getNumSpikes(size_t i, const double &t) {
        size_t c_id = i-1;
//        std::cout << "SpMat read access: " << c_id << ":" << (int)(t/dt) << "\n";
        int n_spikes = sp_spikes(c_id, (int)(t/dt));
//        std::cout << "ok: " << n_spikes  << "\n";
        return n_spikes;
    }
    void push_back(size_t id, const double &t) {
        size_t c_id = id-1;
//        std::cout << "SpMat write access: " << c_id << ":" << (int)(t/dt) << "\n";
        sp_spikes(c_id, (int)(t/dt)) = 1;
//        std::cout << "ok\n";
        sp.push_back( TSpikePair(id, t) );
    }
    arma::sp_mat sp_spikes;    
    const double &dt;
    TVecSpikes sp;
};


class NetSimOld {
public:    
    NetSimOld(List &net_) : net(net_), active_ids(net.size(), arma::fill::zeros) {}
    int getNumSpikes(size_t i, const double &t, const double &dt) {
        size_t c_id = i-1;
        
        int num_sp = 0;
        NumericVector sp = as<NumericVector>(net[c_id]);
        for(size_t sp_i = active_ids(c_id); sp_i < sp.size(); sp_i++) {
            if (sp[sp_i] >= t-dt) {
                if(sp[sp_i] < t) {
                    // spike in (t-dt, t]
                    num_sp += 1;
                } else {
                    // this is spike in future;
                    break;
                }
            } else {
                active_ids(c_id)+=1; // this is spike in past
            }
        }
        return num_sp;
    }
    void push_back(size_t id, const double &t) {
        size_t c_id = id-1;
        NumericVector sp = net[c_id];
        sp.push_back(t);
        net[c_id] = sp;
    }
    List net;
    arma::ivec active_ids;
};


#endif
