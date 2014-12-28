
#include "mod.h"


RCPP_MODULE(snnMod) {
    Rcpp::class_<RSim>("RSim")
    .constructor<RConstants*, size_t>()
    .method("print", &RSim::print, "Print Sim instance")
    .method("run", &RSim::run, "Run simulation")
    .method("getSpikes", &RSim::getSpikes, "get spikes")
    .method("getModel", &RSim::getModel, "get model")
    .method("getStat", &RSim::getStat, "get stat")
    .method("collectStat", &RSim::collectStat, "turn on collect stat")
    .method("setInputSpikesList", &RSim::setInputSpikesList, "Set LabeledSpikesList as input spikes")
    ;
    Rcpp::class_<RConstants>("RConstants")
    .constructor<std::string>()
    .method("print", &RConstants::print, "Print constants")
    ;
    Rcpp::class_<RProto>("RProto")
    .constructor<std::string>()
    .method("read", &RProto::read, "Reading protobuf")
    .method("write", &RProto::write, "Write protobuf")
    .method("print", &RProto::print, "Print proto instance")
    ;
}
