
#include "postprocess.h"
#include "gauss_legendre_vec.h"

double gaussian_kernel(arma::vec s, double sigma) {
   return arma::accu( (1/sqrt(2*arma::datum::pi*pow(sigma,2)))*exp( -arma::pow(s, 2)/(2*pow(sigma,2)) ));
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
SEXP kernelWindow_spikes(List d, const List kernel_options) {
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
    return List::create(Named("data") = out, Named("label") = d["label"]);
}

// correlation stuff:

struct TIntDataVCorr {
    TIntDataVCorr(const List &data1_c, const List &data2_c, const double &sigma_c) : data1(data1_c), data2(data2_c), sigma(sigma_c) {}
    double sigma;
    List data1;
    List data2;
};

arma::vec integrand_kernel_corr_vec(const arma::vec &t, void *int_data) {
    TIntDataVCorr *d = (TIntDataVCorr*)int_data;
    arma::vec out(d->data1.size()*d->data2.size(), arma::fill::zeros);
    for(size_t el_i=0; el_i<d->data1.size(); el_i++) {
        for(size_t el_j=0; el_j<d->data2.size(); el_j++) {
            int ind = el_j+el_i*d->data1.size(); // Column major Tom to ground control
            out(ind) = gaussian_kernel( t(ind) - as<arma::vec>(d->data1[el_i]), d->sigma)*gaussian_kernel( t(ind) - as<arma::vec>(d->data2[el_j]), d->sigma);
        }
    }
    return out;
}

arma::mat spikes_corr(const List &d1, const List &d2, const List &kernel_options) {
    const double &sigma = as<double>(kernel_options["sigma"]);
    const double &T0 = as<double>(kernel_options["T0"]);
    const double &Tmax = as<double>(kernel_options["Tmax"]);
    const int &quad = as<int>(kernel_options["quad"]);

    TIntDataVCorr id(d1, d2, sigma);
    arma::vec Kv = gauss_legendre_vec(quad, integrand_kernel_corr_vec, d1.size()*d2.size(), (void*)&id, T0, Tmax);
    arma::mat K(Kv.memptr(), d1.size(), d2.size());
    return K;
}


// [[Rcpp::export]]
SEXP kernelPass_autoCorr(List d, const List kernel_options) {
    List data = d["data"];
    return List::create(Named("data") = spikes_corr(data, data, kernel_options), Named("label") = d["label"]);
}

// [[Rcpp::export]]
SEXP kernelPass_corr(List d1, List d2, const List kernel_options) {
    List data1 = d1["data"];
    List data2 = d2["data"];
    return List::create(Named("data") = spikes_corr(data1, data2, kernel_options));
}



arma::vec integrand_kernel_cross_neuron_vec(const arma::vec &t, void *int_data) {
    TIntDataVCorr *d = (TIntDataVCorr*)int_data;
    arma::vec out(d->data1.size(), arma::fill::zeros);
    for(size_t el_i=0; el_i<d->data1.size(); el_i++) {
        out(el_i) = gaussian_kernel( t(el_i) - as<arma::vec>(d->data1[el_i]), d->sigma)*gaussian_kernel( t(el_i) - as<arma::vec>(d->data2[el_i]), d->sigma);
    }
    return out;
}

// [[Rcpp::export]]
SEXP kernelPass_crossNeurons(List d1, List d2, const List kernel_options) {
    List data1 = d1["data"];
    List data2 = d2["data"];

    if(data1.size() != data2.size()) {
        printf("Two input lists must have identical size!\n");
        return R_NilValue;
    }
    
    const double &sigma = as<double>(kernel_options["sigma"]);
    const double &T0 = as<double>(kernel_options["T0"]);
    const double &Tmax = as<double>(kernel_options["Tmax"]);
    const int &quad = as<int>(kernel_options["quad"]);

    TIntDataVCorr id(data1, data2, sigma);
    arma::vec K = gauss_legendre_vec(quad, integrand_kernel_cross_neuron_vec, data1.size(), (void*)&id, T0, Tmax);
    
    return List::create(Named("data") = K, Named("label1") = d1["label"], Named("label2") = d2["label"]);
}

