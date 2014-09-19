#ifndef CONV_UTIL_H
#define CONV_UTIL_H


#include <Rcpp.h>


extern "C" {
    #include <util/spikes_list.h>
}


SpikesList* RListToSpikesList(Rcpp::List l);
Rcpp::List SpikesListToRList(SpikesList *sl);
Rcpp::List MatrixVectorToRList(pMatrixVector *mv);
Rcpp::NumericMatrix MatrixToRMatrix(pMatrix *m);
Rcpp::NumericMatrix DoublesAllocToRMatrix(double **v, int *sizes, int N);

#endif
