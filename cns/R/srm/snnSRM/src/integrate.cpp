#include <Rcpp.h>
#include <RcppArmadillo.h>

#include "gauss_legendre.h"
#include "base.h"
#include "neuron.h"
#include "grad_funcs.h"
#include "romberg.h"

using namespace Rcpp;

double integrand(double t, void* data) {
  const SSynInput &ssyn = *(SSynInput*)data;
  return p_stroke(t, ssyn.si) * grab_epsp(t, ssyn);
}


//#define DEBUG
#define GAUSS_QUAD 512
// [[Rcpp::export]]
SEXP integrateSRM(const List constants, const List int_options, const IntegerVector neuron_id, 
                  const IntegerVector neuron_id_conn, const NumericVector neuron_w, const List net) {
    
    const double &T0 = as<double>(int_options["T0"]);
    const double &Tmax = as<double>(int_options["Tmax"]);
    SInput si(constants, neuron_id, neuron_id_conn, neuron_w, net);

#ifdef DEBUG  
    IntegerVector t = seq_len(Tmax);
    NumericMatrix out_ps(si.id_conn.size(), t.size());
    NumericMatrix out_epsp(si.id_conn.size(), t.size());
#endif    
    NumericVector int_values(si.id_conn.size());
    for(size_t syn_id=0; syn_id<si.id_conn.size(); syn_id++) {
      SSynInput ssyn(si.id_conn[syn_id], si);
      int_values[syn_id] = - gauss_legendre(GAUSS_QUAD, integrand, (void*)&ssyn, T0, Tmax);
//      int_values[syn_id] = - integrateRomberg(integrand, (void*)&ssyn, T0, Tmax, 10, 10);
#ifdef DEBUG          
      for(size_t ti=0; ti<t.size(); ti++) {
        out_ps(syn_id,ti) = p_stroke(t[ti], ssyn.si);
        out_epsp(syn_id,ti) = grab_epsp(t[ti], ssyn);   
      }
#endif      
    } 
#ifdef DEBUG     
    NumericVector out_u(t.size());
    for(size_t ti=0; ti<t.size(); ti++) {
      out_u(ti) = u(t[ti], si);
    }    
    return List::create(Named("p_stroke") = out_ps, Named("grab_epsp") = out_epsp, Named("u") = out_u);
#endif
    return int_values;
}    

struct SIntData {
    SIntData(const List constants_c,  const List int_options_c, const IntegerVector neurons_id_c, const List neurons_id_conn_c, const List neurons_w_c, const List net_c) : constants(constants_c),  int_options(int_options_c), neurons_id(neurons_id_c), neurons_id_conn(neurons_id_conn_c), neurons_w(neurons_w_c), net(net_c) {}
    const List &constants;
    const List &int_options;
    const IntegerVector &neurons_id;
    const List &neurons_id_conn;
    const List &neurons_w;
    const List &net;
};

arma::vec integrand_vec(double t, void *data) {
   arma::vec out(100);
   out.fill(3.14);
   return out;
}



SEXP integrateSRM_vec(const List constants,  const List int_options, const IntegerVector neurons_id, const List neurons_id_conn, const List neurons_w, const List net) {
    if(neurons_id.size() != neurons_id_conn.size()) {
        printf("Error. length(w) != length(id_conn).\n");
        return 0;
    }
    SIntData sint_d(constants,int_options,neurons_id,neurons_id_conn,neurons_w, net);
    arma::vec out = integrand_vec(5, (void*)&SIntData);
    return List::create(Named("out") = out);
}
