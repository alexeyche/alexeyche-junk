#ifndef NEURON_FUNCS
#define NEURON_FUNCS

#include <RcppArmadillo.h>

using namespace Rcpp;


double probf(const double &u, const List &c);

arma::vec C_calc(bool Yspike, double p, double u, arma::vec  epsps, const List &constants);
double B_calc(bool Yspike, double p, double pmean, const List &constants);
arma::vec ratecalc(const arma::vec &weights, const double &ws4);

#endif
