
#include "RSim.h"

class RSim {
public:
    RSim() {}
    
    NumericVector v;
};

RCPP_MODULE(RsnnMod){
    class_<RSim>( "RSim" )
    .field("v", &RSim::v, "test")
    ;
}    
