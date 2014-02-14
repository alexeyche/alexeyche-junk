#ifndef NEURON_H
#define NEURON_H

#include "base.h"

#include <Rcpp.h>
using namespace Rcpp;

#define EPSP_WORK_WINDOW 100
#define NU_WORK_WINDOW 100
double nu(const double &s, const List &c);
double epsp(const double s, const List &c);
double u(const double &t, const SInput &si);
double g(const double &g, const List &c);
size_t binary_search(const double &t, const NumericVector &y);
double binary_search_value(const double &t, const NumericVector &y);
double a(const double &s, const List &c);

List simNeurons(const double t, const List &constants, Reference &neurons, const List &net);
NumericVector simNeurons_old(const double t, const List &constants, Reference &neurons, const List &net);

#endif
