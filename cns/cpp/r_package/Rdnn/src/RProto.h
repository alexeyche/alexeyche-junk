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
            if (!o) {
                ERR("Can't read protofile " << protofile << "\n");
            }
            vector<SerializableBase*> obj;
            obj.push_back(o);
            while(true) {
                SerializableBase* o = str.readObject();
                if(!o) break;
                obj.push_back(o);
            }
            
            if(obj.size() == 1) { 
                values = convertToList(o);
            } else {
                vector<Rcpp::List> ret;
                for(auto &o: obj) {
                    ret.push_back(convertToList(o));                    
                }
                values = Rcpp::wrap(ret);
            }
            for(auto& o: obj) {
                Factory::inst().deleteLast();
            }
        }
        return values;
    }
    void write(Rcpp::List &o, const string& name) {
        SerializableBase* b = convertBack(o, name);
        ofstream f(protofile);
        Stream str(f, Stream::Binary);
        str.writeObject(b);
    }
    
    static Rcpp::List convertToList(SerializableBase *o) {
        Rcpp::List out;
        if(o->name() == "Statistics") {
            Statistics *od = dynamic_cast<Statistics*>(o);
            if(!od) { ERR("Can't cast"); }
            StatisticsInfo info = od->getInfo();
            for(auto &name: info.stat_names) {
                out[name] = Rcpp::wrap(od->getStats()[name].values);
            }
        } 
        if(o->name() == "SpikesList") {
            SpikesList *od = dynamic_cast<SpikesList*>(o);
            if(!od) { ERR("Can't cast"); }
            
            vector<vector<double>> sp; 
            for(auto &seq : od->seq) {
                sp.push_back(seq.values);
            }
            out = Rcpp::wrap(sp);
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
