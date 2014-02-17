#ifndef GAUSS_LEGENDRE_VEC_H
#define GAUSS_LEGENDRE_VEC_H

#include "gauss_legendre.h"
#include <RcppArmadillo.h>

arma::vec gauss_legendre_vec(int n, arma::vec (*f)(const arma::vec&,void*), int numDim, void* data, double a, double b);

#endif
