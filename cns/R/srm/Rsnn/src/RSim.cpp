
#include "RSim.h"

class RSim {
public:
    RSim(size_t i) {
        RNGScope scope;
        v = runif(i, 0, 1);
    }
    
    NumericVector v;
};

RCPP_MODULE(snnMod) {
    class_<RSim>("RSim")
    .constructor<size_t>()
    .field("v", &RSim::v, "test")
    ;
}    
