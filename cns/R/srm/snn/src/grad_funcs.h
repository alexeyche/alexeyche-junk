#ifndef GRAD_FUNCS_H
#define GRAD_FUNCS_H

#include "base.h"
#include "neuron.h"

double p_stroke(const double &t, const SInput &si);
double grab_epsp(const double &t, const SSynInput &ssyn);
NumericVector C_calc(bool Yspike, double p, NumericVector epsps);
double B_calc(bool Yspike, double p, double pmean, const List &constants);

#endif
