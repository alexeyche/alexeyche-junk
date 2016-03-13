#include "mod.h"


RCPP_MODULE(dnnMod) {
    Rcpp::class_<TProto>("TProto")
    .constructor()
    .method("read", &TProto::ReadFromFile, "Reading protobuf")
    .method("write", &TProto::WriteToFile, "Write protobuf")
    .method("print", &TProto::Print, "Print debug info")
    ;
}
