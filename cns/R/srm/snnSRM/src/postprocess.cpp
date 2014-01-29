
#include "postprocess.h"
#include "gauss_legendre_vec.h"



//gaussian_kernel = function(s, sigma) {
//  sum((1/sqrt(2*pi*sigma^2))*exp(-(s^2)/(2*sigma^2)))
//}

double gaussian_kernel(arma::vec s, double sigma) {
   return arma::accu( (1/sqrt(2*arma::datum::pi*pow(sigma,2)))*exp( -arma::pow(s, 2)/(2*pow(sigma,2)) ));
}

arma::vec gaussian_kernel_mat(arma::mat s, double sigma) {
   return arma::sum( (1/sqrt(2*arma::datum::pi*pow(sigma,2)))*exp( -arma::pow(s, 2)/(2*pow(sigma,2)) ), 1);
}

struct TIntData {
    double sigma;
    arma::vec data;
};

struct TIntDataV {
    TIntDataV(const List &data_c, const double &sigma_c) : data(data_c), sigma(sigma_c) {}
    double sigma;
    List data;
};


double integrand_kernel(double t, void *int_data) {
    TIntData *d = (TIntData*)int_data;
    return gaussian_kernel(t - d->data, d->sigma) ;
}

arma::vec integrand_kernel_vec(const arma::vec &t, void *int_data) {
    TIntDataV *d = (TIntDataV*)int_data;
    arma::vec out(d->data.size(), arma::fill::zeros);
    for(size_t el_i=0; el_i<d->data.size(); el_i++) {
        out(el_i) = gaussian_kernel( t(el_i) - as<arma::vec>(d->data[el_i]), d->sigma);
    }
    return out;
}

// [[Rcpp::export]]
SEXP kernelPass_spikes(List d, const List kernel_options) {
    List data = d["data"];
    const double &sigma = as<double>(kernel_options["sigma"]);
    const double &window= as<double>(kernel_options["window"]);
    const double &T0 = as<double>(kernel_options["T0"]);
    const double &Tmax = as<double>(kernel_options["Tmax"]);
    const int &quad = as<int>(kernel_options["quad"]);
    TIntDataV id(data, sigma);
    
    arma::vec t = arma::linspace<arma::vec>(T0, Tmax, (Tmax-T0)/window);
    arma::mat out(data.size(), t.n_elem, arma::fill::zeros);
    for(size_t ti=1; ti<t.n_elem; ti++) {
        out.col(ti) = gauss_legendre_vec(quad, integrand_kernel_vec, data.size(), (void*)&id, t(ti-1), t(ti));
    }
    return List::create(Named("data") = out, Named("label") = d["label"], Named("trial") = d["trial"]);
}

// [[Rcpp::export]]
SEXP kernelPass_stat(List d, const List kernel_options) {
    arma::mat data = d["data"];
    const double &sigma = as<double>(kernel_options["sigma"]);
    const double &window= as<double>(kernel_options["window"]);
    const double &T0 = as<double>(kernel_options["T0"]);
    const double &Tmax = as<double>(kernel_options["Tmax"]);
    const int &quad = as<int>(kernel_options["quad"]);
    
    int steps = data.n_cols/window;
    
    arma::mat out(data.n_rows, steps, arma::fill::zeros);
    
    size_t out_it = 0;
    for(int ti=window; ti<data.n_cols; ti+=window) {
        out.col(out_it) = gaussian_kernel_mat( data(arma::span::all, arma::span(ti-window, ti)), sigma);
        out_it++;
    }
    return List::create(Named("data") = out, Named("label") = d["label"], Named("trial") = d["trial"]);
}
