
#include "mod.h"


RCPP_MODULE(dnnMod) {
    Rcpp::class_<RSim>("RSim")
    .constructor<RConstants*>()
    .method("print", &RSim::print, "Print Sim instance")
    .method("run", &RSim::run, "Run simulation")
    .method("getStat", &RSim::getStat, "get stat")
    .method("setTimeSeries", &RSim::setTimeSeries, "Setting time series to object")
    .method("setInputSpikes", &RSim::setInputSpikes, "Setting spikes list to object")
    .method("getSpikes", &RSim::getSpikes, "get spikes")
    .method("getModel", &RSim::getModel, "get model")
    .method("saveModel", &RSim::saveModel, "save mode")
//    .method("collectStat", &RSim::collectStat, "turn on collect stat")
//    .method("setInputSpikesList", &RSim::setInputSpikesList, "Set LabeledSpikesList as input spikes")
    ;
    Rcpp::class_<RConstants>("RConstants")
    .constructor<std::string>()
    .method("print", &RConstants::print, "Print constants")
    ;
    Rcpp::class_<RProto>("RProto")
    .constructor<std::string>()
    .method("read", &RProto::read, "Reading protobuf")
    .method("rawRead", &RProto::rawRead, "Reading protobuf without simplifying")
    .method("write", &RProto::write, "Write protobuf")
    .method("print", &RProto::print, "Print proto instance")
    ;
    Rcpp::class_<RMatchingPursuit>("RMatchingPursuit")
    .constructor<const Rcpp::List>()
    .method("run", &RMatchingPursuit::run, "Running an algorithm")
    .method("setFilter", &RMatchingPursuit::setFilter, "Set filter")
    .method("setConf", &RMatchingPursuit::setConf, "Set conf")
    .method("getFilter", &RMatchingPursuit::getFilter, "Get filter")
    .method("print", &RMatchingPursuit::print, "Print mpl instance")
    .method("restore", &RMatchingPursuit::restore, "Restore time series")
    ;
//    Rcpp::class_<RGammatoneFB>("RGammatoneFB")
//    .constructor()
//    .method("calc", &RGammatoneFB::calc, "Run calculations")
//    .method("print", &RGammatoneFB::print, "Print instance")
//    ;
}
