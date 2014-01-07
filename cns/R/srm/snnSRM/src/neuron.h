#ifndef NEURON_H
#define NEURON_H

#include "base.h"

#include <Rcpp.h>
using namespace Rcpp;

#define EPSP_WORK_WINDOW 50
#define NU_WORK_WINDOW 50
double nu(const double &s, const List &c);
double epsp(const double s, const List &c);
double u(const double &t, const SInput &si);

#endif