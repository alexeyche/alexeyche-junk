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
        TimeSeriesInfo ts_info;
        if( (name == "TimeSeries") || (name == "SpikesList") ) {
            if(list.containsElementNamed("ts_info")) {
                Rcpp::List ts_info_l = list["ts_info"];
                ts_info.labels_ids = Rcpp::as<vector<size_t>>(ts_info_l["labels_ids"]);
                ts_info.unique_labels = Rcpp::as<vector<string>>(ts_info_l["unique_labels"]);
                ts_info.labels_timeline = Rcpp::as<vector<size_t>>(ts_info_l["labels_timeline"]);
            }
        }
        if(name == "TimeSeries") {
            TimeSeries* ts = Factory::inst().createObject<TimeSeries>(name);
            ts->data.values = Rcpp::as<vector<double>>(list["values"]);
            ts->info = ts_info;
            return ts;
        }
        if(name == "SpikesList") {
            Rcpp::List spikes = list["values"];
            SpikesList *sl = Factory::inst().createObject<SpikesList>("SpikesList");

            for(auto &sp_v: spikes) {
                SpikesSequence sp_seq;
                sp_seq.values = Rcpp::as<vector<double>>(sp_v);
                sl->seq.push_back(sp_seq);
            }
            sl->ts_info = ts_info;
            return sl;
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
