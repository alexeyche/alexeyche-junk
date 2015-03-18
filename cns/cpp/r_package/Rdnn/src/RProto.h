#ifndef RPROTO_READ_H
#define RPROTO_READ_H

#include <dnn/io/stream.h>

#undef PI
#define STRICT_R_HEADERS
#include <Rcpp.h>

#include "common.h"

using namespace dnn;

class RProto {
public:
    RProto(std::string _protofile) : protofile(_protofile) {
    }
    Rcpp::List& read() {
        if(values.size() == 0) {
            if(getFileSize(protofile) == 0) return values;

            ifstream f(protofile);
            Stream str(f, Stream::Binary);
            SerializableBase* o = str.readObject();
            values = convertToList(o);
        }
        return values;
    }
    void write(Rcpp::List &o) {
    }
    
    static Rcpp::List convertToList(const SerializableBase *o) {
        Rcpp::List out;
        if(o->name() == "Statistics") {
            const Statistics *st = dynamic_cast<const Statistics*>(o);
            if(!st) { ERR("Can't cast"); }
            
            for(auto &p: st->stats) {
                out[p.first] = Rcpp::wrap(p.second.vals);
            }
        } 
        return out;
    } 
    void print() {
        cout << "RProto instance. run instance$read() method to read protobuf\n";
    }

    Rcpp::List values;
    std::string protofile;
};



#endif
