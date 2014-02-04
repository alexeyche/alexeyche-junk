//#include <Rcpp.h>

// [[Rcpp::depends("RcppArmadillo")]]

#include <RcppArmadillo.h>

#include "gauss_legendre.h"
#include "gauss_legendre_vec.h"
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

arma::vec integrand_vec(const arma::vec &t, void *data) {
    SIntData *sint_d = (SIntData*)data;
        
    std::vector<double> out;
    out.reserve(t.size());
    size_t ti = 0;
    for(size_t it = 0; it< sint_d->neurons_id.size(); it++) {
        IntegerVector id(1); 
        id[0] = sint_d->neurons_id[it];
        IntegerVector id_conn(sint_d->neurons_id_conn[it]);
        NumericVector w(sint_d->neurons_w[it]);
        SInput si(sint_d->constants, id, id_conn, w, sint_d->net);
       
        arma::vec out_n(id_conn.size(), arma::fill::zeros);
        for(size_t syn_id=0; syn_id<id_conn.size(); syn_id++) {
            const double &cur_t = t(ti);
            ti++;
            const NumericVector &sp(sint_d->net[ id_conn[syn_id]-1 ]);
            for(int spike_id=sp.size()-1; spike_id>=0; spike_id--) {
                double s = cur_t - sp[spike_id];
                if(s > EPSP_WORK_WINDOW) break;
                if(s <= 0) continue;      
               
                out_n(syn_id) += epsp(s, sint_d->constants); 
            } 
            out_n(syn_id) *= p_stroke(cur_t, si);
        }    
        out.insert(out.end(), out_n.begin(), out_n.end());
    }   
    return out;
}


// [[Rcpp::export]]
SEXP integrateSRM_vec(const List constants,  const List int_options, const IntegerVector neurons_id, const List neurons_id_conn, const List neurons_w, const List net) {
    if(neurons_id.size() != neurons_id_conn.size()) {
        printf("Error. length(w) != length(id_conn).\n");
        return 0;
    }
    const double &T0 = as<double>(int_options["T0"]);
    const double &Tmax = as<double>(int_options["Tmax"]);
    const int &dim = as<int>(int_options["dim"]);    
    const int &quad = as<int>(int_options["quad"]);    

    SIntData sint_d(constants,int_options,neurons_id,neurons_id_conn,neurons_w, net);
    arma::vec out = gauss_legendre_vec(quad, integrand_vec, dim, (void*)&sint_d, T0, Tmax);
    return List::create( Named("out") = out);
}
