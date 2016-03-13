#ifndef RPROTO_READ_H
#define RPROTO_READ_H

#include <dnn/util/ts/time_series.h>

#include <fstream>

#undef PI
#define STRICT_R_HEADERS
#include <Rcpp.h>

#include "common.h"

using namespace NDnn;

class TProto {
public:

    template <typename T>
    Rcpp::List Translate(const T& ent);

	template <typename T>
    T TranslateBack(const Rcpp::List& l);


    template <typename T>
    bool ReadEntity(std::istream& istr, Rcpp::List& l) {
    	typename T::TProto pb;
    	if (pb.ParseFromIstream(&istr)) {
    		T ent;
    		ent.Deserialize(pb);
    		l = Translate<T>(ent);
    		return true;
    	}
    	return false;
    }

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
