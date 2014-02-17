//#include <Rcpp.h>

// [[Rcpp::depends("RcppArmadillo")]]

#include <RcppArmadillo.h>

#include "gauss_legendre.h"
#include "gauss_legendre_vec.h"
#include "base.h"
#include "neuron.h"
#include "grad_funcs.h"
#include "postprocess.h"

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
SEXP integrateSRM_epsp_pstroke(const List constants,  const List int_options, const IntegerVector neurons_id, const List neurons_id_conn, const List neurons_w, const List net) {
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

struct TProbNoFireData {
    TProbNoFireData(Reference &n, const List &c, const List &netc) : neurons(n), constants(c), net(netc) {}
    Reference &neurons;
    const List &constants;
    const List &net;
};

arma::vec integrand_prob(const arma::vec &t, void *data) {
  TProbNoFireData *d = (TProbNoFireData*)data;
  const IntegerVector ids = as<const IntegerVector>(d->neurons.field("ids"));
  const List weights = as<const List>(d->neurons.field("weights"));
  const List id_conns = as<const List>(d->neurons.field("id_conns"));
  NumericVector p_val(t.n_elem);
  for(size_t it = 0; it<t.n_elem; it++) {
      IntegerVector id(1); 
      id[0] = ids[it];
      IntegerVector id_conn(id_conns[it]);
      NumericVector w(weights[it]);
      SInput si(d->constants, id, id_conn, w, d->net);
      p_val[it] = g(u(t[it], si), d->constants);
  }
  return(p_val);
}

#define DEFAULT_QUAD 256

// [[Rcpp::export]]
List probNoFire(const double T0, const double Tmax, Reference neurons, const List net, const List constants) {
    TProbNoFireData d(neurons, constants, net);
    arma::vec integral = gauss_legendre_vec(DEFAULT_QUAD, integrand_prob, as<int>(neurons.field("len")), (void*)&d, T0, Tmax);
    return List::create(Named("out") = arma::exp(-integral));
}

// [[Rcpp::export]]
List probInt(const double T0, const double Tmax, Reference neurons, const List net, const List constants) {
    TProbNoFireData d(neurons, constants, net);
    arma::vec integral = gauss_legendre_vec(DEFAULT_QUAD, integrand_prob, as<int>(neurons.field("len")), (void*)&d, T0, Tmax);
    return List::create(Named("out") = integral);
}

arma::vec integrand_vec_epsp(const arma::vec &t, void *data) {
    SIntData *sint_d = (SIntData*)data;
        
    std::vector<double> out;
    out.reserve(t.size());
    size_t ti = 0;
    for(size_t it = 0; it< sint_d->neurons_id.size(); it++) {
//        printf("ti: %zu\n", ti);
        IntegerVector id(1); 
        id[0] = sint_d->neurons_id[it];
        IntegerVector id_conn(sint_d->neurons_id_conn[it]);
        NumericVector w(sint_d->neurons_w[it]);
        const NumericVector &y(sint_d->net[id[0]-1]);

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
                double suppr = 1;
                if(y.size() > 0) {
                    suppr = a(sp[spike_id] - binary_search(cur_t, y), sint_d->constants);
                }
                out_n(syn_id) += epsp(s, sint_d->constants)*suppr; 
            } 
        }    
        out.insert(out.end(), out_n.begin(), out_n.end());
    }   
    return out;
}

//#define TEST

// [[Rcpp::export]]
SEXP integrateSRM_epsp(Reference neurons, const List int_options, const List net, const List constants) {
    const double &T0 = as<double>(int_options["T0"]);
    const double &Tmax = as<double>(int_options["Tmax"]);
    const int &dim = as<int>(int_options["dim"]);    
    const int &quad = as<int>(int_options["quad"]);    

    const IntegerVector ids = as<const IntegerVector>(neurons.field("ids"));
    const List weights = as<const List>(neurons.field("weights"));
    const List id_conns = as<const List>(neurons.field("id_conns"));
    if(ids.size() != id_conns.size()) {
        printf("Error. length(w) != length(id_conn).\n");
        return 0;
    }
    SIntData sint_d(constants, int_options, ids, id_conns, weights, net);
#ifdef TEST    
    arma::vec t = arma::linspace<arma::vec>(T0, Tmax, (Tmax-T0)/0.5);
    arma::mat testout(dim, t.n_elem);
    for(size_t ti=0; ti<t.n_elem; ti++) {
        arma::vec curt(dim);    
        curt.fill(t[ti]);
        testout.col(ti) = integrand_vec_epsp(curt, (void*)&sint_d);
    }
#endif    
    arma::vec out = gauss_legendre_vec(quad, integrand_vec_epsp, dim, (void*)&sint_d, T0, Tmax);
#ifdef TEST
    return List::create( Named("out") = out, Named("testout") = testout);
#else
    return List::create( Named("out") = out);
#endif    
}





