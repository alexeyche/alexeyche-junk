#ifndef SIM_H
#define SIM_H

#include "neuron.h"

SEXP simLayers(const List sim_options, const List constants, List layers, List net);

#endif
