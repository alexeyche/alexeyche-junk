#ifndef CONV_UTIL_H
#define CONV_UTIL_H


#include <Rcpp.h>


extern "C" {
    #include <util/spikes_list.h>
}


SpikesList* RListToSpikesList(Rcpp::List l);

#endif
