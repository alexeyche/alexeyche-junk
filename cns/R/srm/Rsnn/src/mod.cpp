
#include "mod.h"

#include "RConstants.h"
#include "RSim.h"


RCPP_MODULE(snnMod) {
    Rcpp::class_<RSim>("RSim")
    .constructor<RConstants*,unsigned char, size_t>()
    .method("print", &RSim::print, "Print sim")
    .method("printLayers", &RSim::printLayers, "Print detailed information about layers")
    .method("setInputSpikes", &RSim::setInputSpikes, "Set input spikes")
    .method("printConn", &RSim::printConn, "Print sim connection map")
    .method("printInputSpikes", &RSim::printInputSpikes, "Print input spikes queue")
    .method("run", &RSim::run, "Run sim")
    .method("getLayerStat", &RSim::getLayerStat, "Get layer stat")
    .method("W", &RSim::W, "Get List of W matrices")
    ;
    Rcpp::class_<RConstants>("RConstants")
    .constructor<std::string>()
    .field("const_filename", &RConstants::const_filename, "Constants filename")
    .method("setValue", &RConstants::setValue, "Set value of constant with section and name")
    .method("print", &RConstants::print, "Print constants")
    ;
}
