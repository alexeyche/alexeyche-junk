#include "neuron.h"

double epsp_old(const double s, const List &c) {
    if(s<0) return 0;
    return as<double>(c["e0"])*(exp(-s/as<double>(c["tm"]))-exp(-s/as<double>(c["ts"])));
}

double epsp(const double s, const List &c) {
    if(s<0) return 0;
    return exp(-s/as<double>(c["tm"]));
}

double nu(const double &s, const List &c) {
    if(s<0) return 0;
    if(s<as<double>(c["dr"])) return as<double>(c["u_abs"]);
    return as<double>(c["u_abs"])*exp(-(s+as<double>(c["dr"]))/as<double>(c["trf"])) + as<double>(c["u_r"])*exp(-s/as<double>(c["trs"]));
}

double g_old(const double &u, const List &c) {
//   (beta/alpha)*(log(1.1+exp(alpha*(tr-u))) -alpha*(tr-u))  
    return ( as<double>(c["beta"])/as<double>(c["alpha"]) ) * 
                ( log( 1 + exp( as<double>(c["alpha"])*(as<double>(c["tr"])-u))) - 
                                    as<double>(c["alpha"])*(as<double>(c["tr"])-u)) ;
}

double a(const double &s, const List &c) {
    if(s<0) return(0);
    return( 1 - exp( - s/as<double>(c["ta"]) ) );
}

double g(const double &u, const List &c) {
    return( (as<double>(c["pr"]) + (u - as<double>(c["u_rest"]))*as<double>(c["gain_factor"]))/as<double>(c["sim_dim"]) );
}

double stdp(const double &s, const SInput &si) {
    double dw;
    if(s<=0) {
        dw = si.get_c("Aplus")*exp(s/si.get_c("tplus"));
    } else {
        dw = si.get_c("Aminus")*exp(-s/si.get_c("tminus"));
    }
    return(dw);
}

double binary_search(const double &t, const NumericVector &y) {
    const size_t &s = y.size();
    if(s == 0) { printf("Binary search on empty Y\n"); return -9999; }
    if (y[0] > t) { return -9999; } 
    if (y[s-1] <= t) { return y[s-1];}
    size_t first = 0;
    size_t last= s;
    size_t mid = first + (last-first)/2;
    while(first < last) {
        if(t < y[mid]) {
            last = mid;
        } else {
            first = mid+1;
        }
        mid = first + (last - first) / 2;
    }

    last--;
    return y[last];
}

// [[Rcpp::export]]
int test_binary_search(const double t, const NumericVector y) {
    return(binary_search(t, y));
}

double u(const double &t, const SInput &si) {
  const NumericVector &y(si.net[si.id[0]-1]);
  
  double e_syn = 0;
  for(size_t id_it=0; id_it < si.id_conn.size(); id_it++) {
    const int &syn_sp_i = si.id_conn[id_it];
    //printf("syn_sp_i = %d\n", syn_sp_i);
    const NumericVector &syn_sp(si.net[syn_sp_i-1]);
    
    //for(size_t tti=0; tti<syn_sp.size(); tti++) printf("syn_sp[%d] = %f\n", tti, syn_sp[tti]);
    
    for(int sp_it= syn_sp.size()-1; sp_it>=0; sp_it--) {
        if(t-syn_sp[sp_it] > EPSP_WORK_WINDOW) {
            //printf("t-syn_sp[%d] = %f > EPSP_WORK_WINDOW\n", sp_it, t-syn_sp[sp_it]);
            break;
        }
        if(t-syn_sp[sp_it]<0) {
            continue;
        }

        //printf("w[%d] = %f\n", id_it, si.w[id_it]);
        //printf("e_syn_before: %f\n", e_syn);
        double suppr = 1;
        if(y.size() > 0) {
    //        printf("bs: %f \n", binary_search(t, y));
            suppr = a(syn_sp[sp_it] - binary_search(t-0.01, y), si.c);
        }
        e_syn += si.w[id_it] * epsp(t-syn_sp[sp_it], si.c)*suppr;
        //printf("e_syn_after: %f\n", e_syn);
               
    }
    
  }
  return si.get_c("u_rest") + e_syn;
}

arma::vec neuron_epsp_calc(const double &t, const SInput &si) {
  const NumericVector &y(si.net[si.id[0]-1]);
  
  arma::vec epsp_out(si.id_conn.size(), arma::fill::zeros);
  for(size_t id_it=0; id_it < si.id_conn.size(); id_it++) {
    const int &syn_sp_i = si.id_conn[id_it];
    const NumericVector &syn_sp(si.net[syn_sp_i-1]);
    for(int sp_it= syn_sp.size()-1; sp_it>=0; sp_it--) {
        if(t-syn_sp[sp_it] > EPSP_WORK_WINDOW) {
            //printf("t-syn_sp[%d] = %f > EPSP_WORK_WINDOW\n", sp_it, t-syn_sp[sp_it]);
            break;
        }
        if(t-syn_sp[sp_it]<0) {
            continue;
        }

        //printf("w[%d] = %f\n", id_it, si.w[id_it]);
        //printf("e_syn_before: %f\n", e_syn);
        double suppr = 1;
        if(y.size() > 0) {
    //        printf("bs: %f \n", binary_search(t, y));
            suppr = a(syn_sp[sp_it] - binary_search(t-0.01, y), si.c);
        }
        epsp_out(id_it) += epsp(t-syn_sp[sp_it], si.c)*suppr;
        //printf("e_syn_after: %f\n", e_syn);
    }
    
  }
  return epsp_out;
}
// [[Rcpp::export]]
SEXP neuron_epsp(const NumericVector t, const List constants, const IntegerVector neuron_id, 
                  const IntegerVector neuron_id_conn, const NumericVector neuron_w, const List net) {
  SInput si(constants, neuron_id, neuron_id_conn, neuron_w, net);
  arma::mat epsp_out(t.size(), neuron_id_conn.size(), arma::fill::zeros);
  for(size_t ti=0; ti<t.size(); ti++) {
    epsp_out.row(ti) = neuron_epsp_calc(t[ti], si).t();
  }
  return List::create(Named("out") = epsp_out);
}

double u_old(const double &t, const SInput &si) {
  const NumericVector &y(si.net[si.id[0]-1]);
  
  double e_syn = 0;
  for(size_t id_it=0; id_it < si.id_conn.size(); id_it++) {
    const int &syn_sp_i = si.id_conn[id_it];
    //printf("syn_sp_i = %d\n", syn_sp_i);
    const NumericVector &syn_sp(si.net[syn_sp_i-1]);
    
    //for(size_t tti=0; tti<syn_sp.size(); tti++) printf("syn_sp[%d] = %f\n", tti, syn_sp[tti]);
    
    for(int sp_it= syn_sp.size()-1; sp_it>=0; sp_it--) {
        if(t-syn_sp[sp_it] > EPSP_WORK_WINDOW) {
            //printf("t-syn_sp[%d] = %f > EPSP_WORK_WINDOW\n", sp_it, t-syn_sp[sp_it]);
            break;
        }
        if(t-syn_sp[sp_it]<0) {
            continue;
        }

        //printf("w[%d] = %f\n", id_it, si.w[id_it]);
        //printf("e_syn_before: %f\n", e_syn);
        e_syn += si.w[id_it] * epsp(t-syn_sp[sp_it], si.c); 
        //printf("e_syn_after: %f\n", e_syn);
               
    }
    
  }
  double nu_pot = 0;
  for(int yi = y.size()-1; yi>=0; yi--) {
    double s = t - y[yi];
    //printf("nu s of %d = %f\n", yi, s);
    
    if(s < 0.001) continue; //ignoring spike that occurres right now
    
    if(s > NU_WORK_WINDOW) {
      break;
    }
    nu_pot += nu(s, si.c);
    
  }
//  printf("nu_pot %f\n", nu_pot);
  return si.get_c("u_rest") + e_syn + nu_pot;
}


// [[Rcpp::export]]
SEXP USRM(const NumericVector t, const List constants, const IntegerVector neuron_id, 
                  const IntegerVector neuron_id_conn, const NumericVector neuron_w, const List net) {
  //for(List::const_iterator it=constants.begin(); it != constants.end(); ++it) {
  //  printf("%f ",as<double>(*it));
  //}
  //printf("\n");
  SInput si(constants, neuron_id, neuron_id_conn, neuron_w, net);
  NumericVector u_val(t.size());
  for(size_t ti=0; ti<t.size(); ti++) {
    u_val[ti] = u(t[ti], si);
  }
  return u_val;
}



NumericVector simNeurons(const double t, const List &constants, Reference &neurons, const List &net) {
  const IntegerVector ids = as<const IntegerVector>(neurons.field("ids"));
  const List weights = as<const List>(neurons.field("weights"));
  const List id_conns = as<const List>(neurons.field("id_conns"));

  NumericVector u_all(ids.size());
  
  for(size_t it = 0; it<ids.size(); it++) {
    IntegerVector id(1); 
    id[0] = ids(it);
    IntegerVector id_conn(id_conns[it]);
    NumericVector w(weights[it]);
    SInput si(constants, id, id_conn, w, net);
    u_all(it) = u(t, si);
  }
  return u_all;
}


// [[Rcpp::export]]
SEXP USRMs(const NumericVector t, const List constants, const IntegerVector neurons_id, 
                  const List neurons_id_conn, const List neurons_w, const List net) {
  
  if(neurons_id.size() != neurons_id_conn.size()) {
    printf("Error. length(w) != length(id_conn).\n");
    return 0;
  }
  NumericMatrix u_all(neurons_id.size(),t.size());
  for(size_t it = 0; it<neurons_id.size(); it++) {
    IntegerVector id(1); 
    id[0] = neurons_id[it];
    IntegerVector id_conn(neurons_id_conn[it]);
    NumericVector w(neurons_w[it]);
    SInput si(constants, id, id_conn, w, net);
    for(size_t ti=0; ti<t.size(); ti++) {
      u_all(it, ti) = u(t[ti], si);
    }
  }
  return u_all;
}

//=====================================================

List uFull(const double &t, const SInput &si) {
  const NumericVector &y(si.net[si.id[0]-1]);
  
  NumericVector e_syn(si.id_conn.size());
  for(size_t id_it=0; id_it < si.id_conn.size(); id_it++) {
    const int &syn_sp_i = si.id_conn[id_it];
    //printf("syn_sp_i = %d\n", syn_sp_i);
    const NumericVector &syn_sp(si.net[syn_sp_i-1]);
    
    //for(size_t tti=0; tti<syn_sp.size(); tti++) printf("syn_sp[%d] = %f\n", tti, syn_sp[tti]);
    
    for(int sp_it= syn_sp.size()-1; sp_it>=0; sp_it--) {
        if(t-syn_sp[sp_it] > EPSP_WORK_WINDOW) {
            //printf("t-syn_sp[%d] = %f > EPSP_WORK_WINDOW\n", sp_it, t-syn_sp[sp_it]);
            break;
        }
        if(t-syn_sp[sp_it]<0) {
            continue;
        }

        //printf("w[%d] = %f\n", id_it, si.w[id_it]);
        //printf("e_syn_before: %f\n", e_syn);
        double suppr = 1;
        if(y.size() > 0) {
    //        printf("bs: %f \n", binary_search(t, y));
            suppr = a(syn_sp[sp_it] - binary_search(t-0.01, y), si.c);
        }
        e_syn[id_it] += epsp(t-syn_sp[sp_it], si.c)*suppr;
        //printf("e_syn_after: %f\n", e_syn);
               
    }
    
  }
  NumericVector e_syn_w = e_syn * si.w;
  return List::create(Named("u") = si.get_c("u_rest") + std::accumulate(e_syn_w.begin(), e_syn_w.end(), 0.0), Named("epsp") = e_syn);
}

// [[Rcpp::export]]
SEXP USRMsFull(const NumericVector t, const List constants, const IntegerVector neurons_id, 
                  const List neurons_id_conn, const List neurons_w, const List net) {
  
  if(neurons_id.size() != neurons_id_conn.size()) {
    printf("Error. length(w) != length(id_conn).\n");
    return 0;
  }
  List epsps(neurons_id.size());
  NumericVector u_all(neurons_id.size());
  for(size_t it = 0; it<neurons_id.size(); it++) {
    IntegerVector id(1); 
    id[0] = neurons_id[it];
    IntegerVector id_conn(neurons_id_conn[it]);
    NumericVector w(neurons_w[it]);
    SInput si(constants, id, id_conn, w, net);
    List out = uFull(t[0], si);
    u_all(it) = as<double>(out["u"]);
    epsps[it] = out["epsp"];
  }
  return List::create(Named("u") = u_all, Named("epsps") = epsps);
}

