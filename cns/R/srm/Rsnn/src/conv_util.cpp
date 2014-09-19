
#include "conv_util.h"

SpikesList* RListToSpikesList(Rcpp::List l) {
    SpikesList* sl = createSpikesList(l.size());
    for(size_t i=0; i<l.size(); i++) {
        Rcpp::NumericVector v = l[i];
        for(size_t vi=0; vi<v.size(); vi++) {
            TEMPLATE(insertVector,double)(sl->list[i], v[vi]);
        }
    }
    return sl;
}

Rcpp::List SpikesListToRcppList(SpikesList *sl) {
    Rcpp::List l(sl->size);
    for(size_t i=0; i<sl->size(); i++) {
        Rcpp::NumericVector v = l[i];
        for(size_t vi=0; vi<v.size(); vi++) {
            TEMPLATE(insertVector,double)(sl->list[i], v[vi]);
        }
    }
    return sl;
}
