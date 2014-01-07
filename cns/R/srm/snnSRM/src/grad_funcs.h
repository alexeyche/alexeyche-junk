#ifndef GRAD_FUNCS_H
#define GRAD_FUNCS_H

#include "base.h"
#include "neuron.h"

double p_stroke(const double &t, const SInput &si);
double grab_epsp(const double &t, const SSynInput &ssyn);


#endif