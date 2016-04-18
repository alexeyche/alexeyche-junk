#ifndef RPROTO_READ_H
#define RPROTO_READ_H

#include <dnn/base/base.h>
#include <dnn/protos/config.pb.h>

#include <fstream>

#undef PI
#define STRICT_R_HEADERS
#include <Rcpp.h>

#include "common.h"

using namespace NDnn;

class TProto {
public:

    template <typename T>
    static Rcpp::List Translate(const T& ent);

    static Rcpp::List TranslateModel(const NDnnProto::TConfig& config);


	template <typename T>
    static T TranslateBack(const Rcpp::List& l);

    template <typename T>
    void WriteEntity(T&& v, std::ostream& ostr) {
    	typename T::TProto pb = v.Serialize();
    	pb.SerializeToOstream(&ostr);
    }

    Rcpp::List ReadFromFile(TString protofile);

    void WriteToFile(Rcpp::List l, TString protofile);

    void Print() {
        std::cout << "RProto instance. run instance$read() method to read protobuf\n";
    }

};

#endif
