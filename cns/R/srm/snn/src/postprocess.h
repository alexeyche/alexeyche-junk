#ifndef POSTPROCESS_H
#define POSTPROCESS_H

#include "neuron.h"

SEXP kernelWindow_spikes(List d, const List kernel_options);
SEXP kernelPass_autoCorr(List d, const List kernel_options);
SEXP kernelPass_corr(List d1, List d2, const List kernel_options);
SEXP kernelPass_crossNeurons(List d1, List d2, const List kernel_options);

#endif
