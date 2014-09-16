#ifndef NETSIM_H
#define NETSIM_H

//#include <RcppArmadillo.h>


//using namespace Rcpp;
//using namespace std;

#include "sim.h"
class Layer;

struct TConseq {
    TConseq(size_t nid_, size_t syn_id_) : nid(nid_), syn_id(syn_id_) {}
    size_t nid;
    size_t syn_id;
};
struct TSynSpike {
    TSynSpike(double t_, size_t syn_id_) : t(t_), syn_id(syn_id_) {}
    double t;
    size_t syn_id;
};

typedef vector<TSynSpike> TSynSpikes;
typedef vector<TConseq> TConsequents;
typedef vector<TSynSpikes> TNeuronQueue;

class NetSim {
public:
    NetSim(List &net, const vector<Layer*> &layers);
    void prop_spike(size_t id, double t);
    TSynSpikes getSpikes(size_t ni, const double &t, const double &dt);

    TNeuronQueue queue_of_spikes;
    TNeuronQueue queue_of_input_spikes;
    arma::uvec spikes_iter;
    arma::uvec input_spikes_iter;
    
    vector<TConsequents> cons;
    List net;
};


//class NetSimOld2 {
//public:    
//    NetSimOld2(const List &net, const int T_size, const double &dt_) : sp_spikes(net.size(), T_size+1), dt(dt_) {
//        for(size_t sp_i=0; sp_i < net.size(); sp_i++) {
//            NumericVector sp = net[sp_i];
//            for(size_t spike_i=0; spike_i < sp.size(); spike_i++) {
//                if((int)(sp[spike_i]/dt) > T_size) {
//                    break;
//                }
//                sp_spikes(sp_i, (int)(sp[spike_i]/dt)) = 1;
//            }
//        }
////        sp_spikes.print();
//    }
////    arma::vec getNumSpikesV(const arma::uvec &i, const double &t) {
////        arma::uvec c_id = i - 1;
////        std::cout << "SpMat read access: " << c_id << ":" << (int)(t/dt) << "\n";
////        arma::SpSubview<double> all_spikes = sp_spikes.col((int)(t/dt));
////        std::cout << "ok: " << all_spikes  << "\n";
////        arma::vec n_spikes = all_spikes(c_id);
////        return n_spikes;
////    }
//    int getNumSpikes(size_t i, const double &t) {
//        int index = (int)(t/dt);
//        if ((index<0)||(index>(sp_spikes.n_cols-1))) return 0;
//        size_t c_id = i-1;
////        std::cout << "SpMat read access: " << c_id << ":" << (int)(t/dt) << "\n";
//        int n_spikes = sp_spikes(c_id, index);
////        std::cout << "ok: " << n_spikes  << "\n";
//        return n_spikes;
//    }
//    void push_back(size_t id, const double &t) {
//        size_t c_id = id-1;
//        int index = (int)(t/dt);
// //       std::cout << "SpMat write access: " << c_id << ":" << index << "\n";
//        if ((index<0)||(index>=(sp_spikes.n_cols))) return;
//        sp_spikes(c_id, index) = 1;
// //       std::cout << "ok\n";
//        sp.push_back( TSpikePair(id, t) );
//    }
//    arma::sp_mat sp_spikes;    
//    const double &dt;
//    TVecSpikes sp;
//};


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
