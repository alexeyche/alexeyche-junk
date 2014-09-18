
#include "mod.h"

#include "RConstants.h"
#include "RSim.h"


RCPP_MODULE(snnMod) {
    Rcpp::class_<RSim>("RSim")
    .constructor<RConstants*,unsigned char, size_t>()
    .method("print", &RSim::print, "Print sim")
    ;
    Rcpp::class_<RConstants>("RConstants")
    .constructor<std::string>()
    .field("const_filename", &RConstants::const_filename, "Constants filename")
    .method("print", &RConstants::print, "Print constants")
    ;
}
