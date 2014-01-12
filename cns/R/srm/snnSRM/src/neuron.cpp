#include "neuron.h"

double epsp(const double s, const List &c) {
    if(s<0) return 0;
    return as<double>(c["e0"])*(exp(-s/as<double>(c["tm"]))-exp(-s/as<double>(c["ts"])));
}
double nu(const double &s, const List &c) {
    if(s<0) return 0;
    if(s<as<double>(c["dr"])) return as<double>(c["u_abs"]);
    return as<double>(c["u_abs"])*exp(-(s+as<double>(c["dr"]))/as<double>(c["trf"])) + as<double>(c["u_r"])*exp(-s/as<double>(c["trs"]));
}


double u(const double &t, const SInput &si) {
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
      //printf("w[%d] = %f\n", id_it, si.w[id_it]);
      //printf("e_syn_before: %f\n", e_syn);
      e_syn += si.w[id_it] * epsp(t-syn_sp[sp_it], si.c); 
      //printf("e_syn_after: %f\n", e_syn);
    }
  }
  double nu_pot = 0;
  const NumericVector &y(si.net[si.id[0]-1]);
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

