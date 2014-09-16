#ifndef POSTPROCESS_H
#define POSTPROCESS_H

#include <RcppArmadillo.h>

using namespace Rcpp;
using namespace std;


SEXP kernelWindow_spikes(List d, const List kernel_options);
SEXP kernelPass_autoCorr(List d, const List kernel_options);
SEXP kernelPass_corr(List d1, List d2, const List kernel_options);
SEXP kernelPass_crossNeurons(List d1, List d2, const List kernel_options);
double gaussian_kernel(arma::vec s, double sigma);
double gaussian_function(double t, double sigma);

#endif
