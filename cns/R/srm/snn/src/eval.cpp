#include "sim.h"

// [[Rcpp::export]]
SEXP evalNet(const List patterns, const List run_options, const List constants, List layers) {
    const double &T0 = as<double>(run_options["T0"]);
    const double &Tmax = as<double>(run_options["Tmax"]);
    const double &dt = as<double>(run_options["dt"]);    
    const int &evalTrial = as<int>(run_options["evalTrial"]);    
    List sim_options = List::create(Named("T0") = T0, Named("Tmax") = Tmax, Named("dt") = dt, Named("saveStat") = true, Named("learn") = false, Named("seed") = 0);
    
    List net_all(patterns.size()*evalTrial);
    List stat_all(patterns.size()*evalTrial);

    size_t net_all_it = 0;
    for(size_t pi=0; pi<patterns.size(); pi++) {
        List p = as<List>(patterns[pi])["data"]; 
        int net_size=p.size();
        
        for(size_t li=0; li<layers.size(); li++) {
            net_size += as<NumericVector>(as<Reference>(layers[li]).field("len"))[0];
        }
        List net(net_size);
        
        size_t n_el = 0;
        for(; n_el< p.size(); n_el++) {
            net[n_el] = p[n_el];
        }
        
        for(size_t tr_i=0; tr_i < evalTrial; tr_i++) {
            for(size_t n_rest_i=n_el; n_rest_i<net_size; n_rest_i++) {
                net[n_rest_i] = -INFINITY;
            }
            List stat_out = simLayers(sim_options, constants, layers, net);
            
            List net_out_n(net_size-n_el);
            size_t it = 0;
            for(size_t n_rest_i=n_el; n_rest_i<net_size; n_rest_i++) {
                net_out_n[it] = net[n_rest_i];
                it++;
            }
           
            net_all[net_all_it] = List::create(Named("data") = net_out_n, Named("label") = as<List>(patterns[pi])["label"], Named("trial") = tr_i+1);
            stat_all[net_all_it] = List::create(Named("data") = stat_out["stat"], Named("label") = as<List>(patterns[pi])["label"], Named("trial") = tr_i+1);

            net_all_it += 1;
        }
    }
    return List::create(Named("spikes") = net_all, Named("stat") = stat_all);
}

