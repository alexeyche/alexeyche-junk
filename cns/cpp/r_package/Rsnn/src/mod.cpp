
#include "mod.h"

#include "RConstants.h"
#include "RSim.h"

RCPP_MODULE(snnMod) {
    Rcpp::class_<RSim>("RSim")
    .constructor<RConstants*>()
    ;
    Rcpp::class_<RConstants>("RConstants")
    .constructor<std::string>()
    .method("print", &RConstants::print, "Print constants")
    ;
}
