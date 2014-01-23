#include "neuron.h"

// [[Rcpp::export]]
SEXP simLayers(const List sim_options, const List constants, List layers, List net) {

    const double &T0 = as<double>(sim_options["T0"]);
    const double &Tmax = as<double>(sim_options["Tmax"]);
    const double &dt = as<double>(sim_options["dt"]);    
    const bool &saveStat = as<double>(sim_options["saveStat"]);    
    
    arma::vec T = arma::linspace(T0, Tmax, (Tmax-T0)/dt);
    List stat(layers.size());
    if(saveStat) {
        for(size_t li=0; li<layers.size(); li++) {
            stat[li] = NumericMatrix(T.n_elem, as<Reference>(layers[li]).field("len"));
        }
    }
    for(size_t ti=0; ti<T.n_elem; ti++) {
        for(size_t li=0; li<layers.size(); li++) {
            Reference layer = layers[li];
            const IntegerVector ids = as<const IntegerVector>(layer.field("ids"));
            
            const NumericVector u = simNeurons(T(ti), constants, layer, net);
            const arma::vec coins(u.size(), arma::fill::randu); 
            for(size_t ui=0; ui<u.size(); ui++) {
                if(g(u(ui), constants)*dt > coins(ui)) {
                    //printf("spike %d at %f with u = %f  and  p = %f\n", ids[ui], T(ti), u(ui), g(u(ui), constants));
                    NumericVector st = net[ids[ui]-1];
                    st.push_back(T(ti));
                    net[ids[ui]-1] = st;
                }
                if(saveStat) {
                    NumericMatrix layer_stat = stat[li];
                    layer_stat(ti, ui) = u(ui);
                }
            }
        }
    }
    if(saveStat) {
        return List::create(Named("stat") = stat);
    } else {
        return List::create(Named("stat") = R_NilValue);
    }
}

