#include "sim.h"
#include "grad_funcs.h"

// [[Rcpp::export]]
SEXP simLayers(const List sim_options, const List constants, List layers, List net) {

    const double &T0 = as<double>(sim_options["T0"]);
    const double &Tmax = as<double>(sim_options["Tmax"]);
    const double &dt = as<double>(sim_options["dt"]);    
    const bool &saveStat = as<bool>(sim_options["saveStat"]);    
    const double sim_dim = as<double>(constants["sim_dim"]);

    arma::vec T = arma::linspace(T0, Tmax, (Tmax-T0)/dt);
    int neurons_num = 0;
    if(saveStat) {
        for(size_t li=0; li<layers.size(); li++) {
            NumericVector l_vec = as<Reference>(layers[li]).field("len");
            neurons_num += l_vec[0];
        }
    }
    arma::mat Cacc(neurons_num, T.n_elem, arma::fill::zeros);
    arma::mat stat(neurons_num, T.n_elem, arma::fill::zeros);
    arma::mat Bstat(neurons_num, T.n_elem, arma::fill::zeros);
    arma::cube Cstat(net.size(), net.size(), T.n_elem, arma::fill::zeros);

    arma::vec mean_acc(neurons_num, arma::fill::zeros);
    for(size_t ti=0; ti<T.n_elem; ti++) {
        size_t neuron_it = 0;
        for(size_t li=0; li<layers.size(); li++) {
            Reference layer = layers[li];
            const IntegerVector ids = as<const IntegerVector>(layer.field("ids"));
            
            const List sim_out = simNeurons(T(ti), constants, layer, net);
            const NumericVector u = sim_out["u"];
            const List epsps_current = sim_out["epsps"];

            const arma::vec coins(u.size(), arma::fill::randu); 
            for(size_t ui=0; ui<u.size(); ui++) {
                double p = g(u(ui), constants);
                mean_acc(neuron_it) += p; 
                bool Yspike = false;
                if(p*dt > coins(ui)) {
                    //printf("spike %d at %f with u = %f  and  p = %f\n", ids[ui], T(ti), u(ui), g(u(ui), constants));
                    NumericVector st = net[ids[ui]-1];
                    st.push_back(T(ti));
                    net[ids[ui]-1] = st;
                    Yspike = true;
                }
                NumericVector C = C_calc(Yspike, p, epsps_current[ui]);
                double B = B_calc(Yspike, p, mean_acc(neuron_it)/(ti+1), constants);
                if(saveStat) {
                    stat(neuron_it, ti) = p;
                    Bstat(neuron_it, ti) = B;
                    IntegerVector id_conn = as<IntegerVector>(as<List>(layer.field("id_conns"))[ui]);
                    for(size_t idi=0; idi<id_conn.size(); idi++) {
                        Cstat(ids[ui]-1, id_conn[idi]-1, ti) = C[idi];
                    }
                    neuron_it++;
                }
            }
        }
    }
    if(saveStat) {
        return List::create(Named("stat") = List::create(Named("pstat") = stat, Named("Cstat") = Cstat, Named("Bstat") = Bstat) );
    } else {
        return List::create(Named("stat") = R_NilValue);
    }
}

// [[Rcpp::export]]
SEXP simLayers_old(const List sim_options, const List constants, List layers, List net) {

    const double &T0 = as<double>(sim_options["T0"]);
    const double &Tmax = as<double>(sim_options["Tmax"]);
    const double &dt = as<double>(sim_options["dt"]);    
    const bool &saveStat = as<bool>(sim_options["saveStat"]);    
    
    arma::vec T = arma::linspace(T0, Tmax, (Tmax-T0)/dt);
    int neurons_num = 0;
    if(saveStat) {
        for(size_t li=0; li<layers.size(); li++) {
            NumericVector l_vec = as<Reference>(layers[li]).field("len");
            neurons_num += l_vec[0];
        }
    }
    arma::mat stat(neurons_num, T.n_elem, arma::fill::zeros);
    for(size_t ti=0; ti<T.n_elem; ti++) {
        size_t neuron_it = 0;
        for(size_t li=0; li<layers.size(); li++) {
            Reference layer = layers[li];
            const IntegerVector ids = as<const IntegerVector>(layer.field("ids"));
            
            const NumericVector u = simNeurons_old(T(ti), constants, layer, net);
            const arma::vec coins(u.size(), arma::fill::randu); 
            for(size_t ui=0; ui<u.size(); ui++) {
                double p = g(u(ui), constants);
                if(p*dt > coins(ui)) {
                    //printf("spike %d at %f with u = %f  and  p = %f\n", ids[ui], T(ti), u(ui), g(u(ui), constants));
                    NumericVector st = net[ids[ui]-1];
                    st.push_back(T(ti));
                    net[ids[ui]-1] = st;
                }
                if(saveStat) {
                    stat(neuron_it, ti) = p;
                    neuron_it++;
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

