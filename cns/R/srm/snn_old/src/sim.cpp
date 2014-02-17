#include "sim.h"
#include "grad_funcs.h"
#include "integrate.h"

// [[Rcpp::export]]
SEXP simLayers(const List sim_options, const List constants, List layers, List net) {

    const double T0 = as<double>(sim_options["T0"]);
    const double Tmax = as<double>(sim_options["Tmax"]);
    const double dt = as<double>(sim_options["dt"]);    
    const bool saveStat = as<bool>(sim_options["saveStat"]);    
    const bool learn = as<bool>(sim_options["learn"]);    
    
//    const int seed = as<int>(sim_options["seed"]);
//    arma::arma_rng::set_seed(seed);
    
    arma::vec T = arma::linspace(T0, Tmax, (Tmax-T0)/dt);
   
    
    arma::mat Cacc, stat, Bstat, Istat, Clast;
    arma::cube dwstat, Cstat, wstat;
    arma::vec Pxy, Py;
    int Istat_freq;
    if(saveStat) {
        int neurons_num = 0;
        for(size_t li=0; li<layers.size(); li++) {
            NumericVector l_vec = as<Reference>(layers[li]).field("len");
            neurons_num += l_vec[0];
        }       
        Cacc = arma::mat(neurons_num, net.size(), arma::fill::zeros);
        Clast = arma::mat(neurons_num, net.size(), arma::fill::zeros);
        stat = arma::mat(neurons_num, T.n_elem, arma::fill::zeros);
        Bstat = arma::mat(neurons_num, T.n_elem, arma::fill::zeros);
        Istat_freq = 500;
        Istat = arma::mat(neurons_num, ((Tmax-T0)/dt)/Istat_freq, arma::fill::zeros);
        Pxy = arma::vec(neurons_num, arma::fill::ones);
        Py = arma::vec(neurons_num, arma::fill::ones);
        dwstat = arma::cube(neurons_num, net.size(), T.n_elem, arma::fill::zeros);
        wstat = arma::cube(neurons_num, net.size(), T.n_elem, arma::fill::zeros);
        Cstat = arma::cube(neurons_num, net.size(), T.n_elem, arma::fill::zeros);
    }
    
    for(size_t ti=0; ti<T.n_elem; ti++) {
        size_t neuron_it = 0;
        for(size_t li=0; li<layers.size(); li++) {
            Reference layer = layers[li];
            const IntegerVector ids = as<const IntegerVector>(layer.field("ids"));
            const List id_conns = as<List>(layer.field("id_conns"));
            List weights = as<List>(layer.field("weights"));
            NumericVector mean_acc = as<NumericVector>(layer.field("mean_acc"));
            IntegerVector mean_count = as<IntegerVector>(layer.field("mean_count"));

            const List sim_out = simNeurons(T(ti), constants, layer, net);
            const NumericVector u = sim_out["u"];
            const List epsps_current = sim_out["epsps"];
            const List fired_inputs = sim_out["fired_inputs"];

            const arma::vec coins(u.size(), arma::fill::randu); 
            for(size_t ui=0; ui<u.size(); ui++) {
                double p = g(u(ui), constants)*dt;
                bool Yspike = false;
                if(p > coins(ui)) {
                    NumericVector st = net[ids[ui]-1];
                    st.push_back(T(ti));
                    net[ids[ui]-1] = st;
                    Yspike = true;
                }
                mean_acc[ui] += p;
                IntegerVector fired = fired_inputs[ui];
                IntegerVector id_conn = as<IntegerVector>(id_conns[ui]);
                NumericVector weight = as<NumericVector>(weights[ui]);
                
                NumericVector C = C_calc(Yspike, p, epsps_current[ui], constants);
                double pmean = mean_acc[ui]/(mean_count[0]+1);
                double B = B_calc(Yspike, p, pmean, constants);

                NumericVector dw(id_conn.size());
                for(size_t idi=0; idi<id_conn.size(); idi++) {
                    //std::cout << idi << ":" << as<NumericVector>(epsps_current[ui])[idi] << " ";
//                    Cacc(neuron_it, id_conn[idi]-1) += (-Cacc(neuron_it, id_conn[idi]-1) + C[idi])/as<double>(constants["tc"]);
                    Cacc(neuron_it, id_conn[idi]-1) += -Cacc(neuron_it, id_conn[idi]-1)/as<double>(constants["tc"])  + C[idi];
                    dw[idi] = ratecalc(weight[idi], constants)*(Cacc(neuron_it, id_conn[idi]-1)*B - as<double>(constants["weight_decay_factor"])*fired[idi]*weight[idi]);
//                    Clast(neuron_it, id_conn[idi]-1) = Cacc(neuron_it, id_conn[idi]-1);
                }
                //std::cout << "\n";
                if(learn) {
                    weights[ui] = weight + as<double>(constants["added_lrate"])*dw;    
                }
                if(saveStat) {
                    if(Yspike) {
                        Pxy(neuron_it) *= p;
                        Py(neuron_it) *= pmean;
                    } else { 
                        Pxy(neuron_it) *= exp(-p);
                        Py(neuron_it) *= exp(-pmean);
                    }
                    //if((ti>0) && (std::fmod( (ti+1) * dt, Istat_freq) < 0.01)) {
                    //    Istat(neuron_it, (ti+1)*dt/Istat_freq-1) = log(Pxy(neuron_it)/Py(neuron_it));
                    //    Pxy(neuron_it) = 1;
                    //    Py(neuron_it) = 1;
                    //}
                    stat(neuron_it, ti) = p;
                    Bstat(neuron_it, ti) = B;
                    for(size_t idi=0; idi<id_conn.size(); idi++) {
                        Cstat(neuron_it, id_conn[idi]-1, ti) = Cacc(neuron_it, id_conn[idi]-1);
                        dwstat(neuron_it, id_conn[idi]-1, ti) = dw[idi];
                        wstat(neuron_it, id_conn[idi]-1, ti) = weight[idi];
                    }
                    neuron_it++;
                }
            }
            mean_count[0] = mean_count[0] + 1;
            layer.field("mean_count") = mean_count; 
        }
    }
    if(saveStat) {
        return List::create(Named("stat") = List::create(Named("pstat") = stat, Named("Cstat") = Cstat, Named("Bstat") = Bstat, Named("dwstat") = dwstat, Named("wstat") = wstat, Named("Istat") = mean(Istat, 1) ) );
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

