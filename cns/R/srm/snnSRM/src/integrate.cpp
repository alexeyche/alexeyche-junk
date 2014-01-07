#include <Rcpp.h>
#include "gauss_legendre.h"
#include "base.h"
#include "neuron.h"
#include "grad_funcs.h"

using namespace Rcpp;

double integrand(double t, void* data) {
  const SSynInput &ssyn = *(SSynInput*)data;
  return p_stroke(t, ssyn.si) * grab_epsp(t, ssyn);
}

#define GAUSS_QUAD 256
// [[Rcpp::export]]
SEXP integrateSRM(const List constants, const List int_options, const IntegerVector neuron_id, 
                  const IntegerVector neuron_id_conn, const NumericVector neuron_w, const List net) {
    
    const double &T0 = int_options["T0"];
    const double &Tmax = int_options["Tmax"];
    SInput si(constants, neuron_id, neuron_id_conn, neuron_w, net);
    IntegerVector t = seq_len(Tmax);
#ifdef DEBUG    
    NumericMatrix out_ps(si.id_conn.size(), t.size());
    NumericMatrix out_epsp(si.id_conn.size(), t.size());
#endif    
    NumericVector int_values(si.id_conn.size());
    for(size_t syn_id=0; syn_id<si.id_conn.size(); syn_id++) {
      SSynInput ssyn(si.id_conn[syn_id], si);
      int_values[syn_id] = - gauss_legendre(GAUSS_QUAD, integrand, (void*)&ssyn, T0, Tmax);
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
