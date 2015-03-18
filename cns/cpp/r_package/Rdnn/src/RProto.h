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
                out[p.first] = Rcpp::wrap(p.second.values);
            }
        } 
        return out;
    }
    
    template <typename T>
    static T* convertBack(const Rcpp::List &list, const string &name) {
        SerializableBase* o = convertBack(list, name);
        T* oc = dynamic_cast<T*>(o);
        if(!oc) { ERR("Can't cast"); }
        return oc;
    }

    static SerializableBase* convertBack(const Rcpp::List &list, const string &name) {
        if(name == "TimeSeries") {
            TimeSeries* ts = Factory::inst().createTimeSeries();
            ts->data.values = Rcpp::as<vector<double>>(list["values"]);
            return ts;
        }
        ERR("Can't convert " << name );
    } 
    void print() {
        cout << "RProto instance. run instance$read() method to read protobuf\n";
    }

    Rcpp::List values;
    std::string protofile;
};



#endif
