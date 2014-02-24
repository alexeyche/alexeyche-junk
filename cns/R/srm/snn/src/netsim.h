#ifndef NETSIM_H
#define NETSIM_H

#include <RcppArmadillo.h>

using namespace Rcpp;
using namespace std;



class NetSim {
public:    
    NetSim(List &net_) : net(net_), active_ids(net.size(), arma::fill::zeros) {}
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
