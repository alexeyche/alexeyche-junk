#ifndef RPROTO_READ_H
#define RPROTO_READ_H

#include <dnn/io/stream.h>
#include <dnn/util/matrix.h>
#include <R.h>
#include <Rinternals.h>

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
        return _read(true);
    }
    Rcpp::List& rawRead() {
        return _read(false);
    }

    Rcpp::List& _read(bool simplify = true) {
        if(values.size() == 0) {
            if(getFileSize(protofile) == 0) return values;

            ifstream f(protofile);
            vector<SerializableBase*> obj;

                
            try {
                Stream str(f, Stream::Binary);
                
                Factory::inst().registrationOff();
                SerializableBase* o = str.readObject();
                
                if (!o) {
                    ERR("Can't read protofile " << protofile << "\n");
                }
                
                while(o) {
                    obj.push_back(o);
                    o = str.readObject();                    
                }
            } catch(...) {
                ERR("Can't open " << protofile << " for reading\n");
            }

            if((obj.size() == 1)&&(simplify)) { 
                values = convertToList(obj[0]);
                delete obj[0];
            } else {
                vector<Rcpp::List> ret;
                for(auto &o: obj) {
                    Rcpp::List l = convertToList(o);
                    if(l.size()>0) {
                        ret.push_back(l);                    
                    }
                    delete o;
                }
                values = Rcpp::wrap(ret);
            }
            Factory::inst().registrationOn();            
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
            out = Rcpp::List::create(
                  Rcpp::Named("values") = Rcpp::wrap(sp)
                , Rcpp::Named("ts_info") = Rcpp::List::create(
                      Rcpp::Named("labels_ids") = Rcpp::wrap(od->ts_info.labels_ids)
                    , Rcpp::Named("unique_labels") = Rcpp::wrap(od->ts_info.unique_labels)
                    , Rcpp::Named("labels_timeline") = Rcpp::wrap(od->ts_info.labels_timeline)
                )
            );
        }
        if(o->name() == "DoubleMatrix") {
            DoubleMatrix *m = dynamic_cast<DoubleMatrix*>(o);
            if(!m) { ERR("Can't cast"); }
            
            Rcpp::NumericMatrix rm(m->nrow(), m->ncol());
            for(size_t i=0; i<m->nrow(); ++i) {
                for(size_t j=0; j<m->ncol(); ++j) {
                    rm(i,j) = m->getElement(i,j);
                }
            }
            out = Rcpp::List::create(
                Rcpp::Named("DoubleMatrix") = rm
            );
        }
        SpikeNeuronBase* nb = dynamic_cast<SpikeNeuronBase*>(o);
        if(nb) {
            vector<double> weights;
            vector<double> ids_pre;
            for(auto &s: nb->getSynapses()) {
                weights.push_back(s.ref().weight());
                ids_pre.push_back(s.ref().idPre());
            }
            
            out = Rcpp::List::create(
                  Rcpp::Named("xi") = nb->xi()
                , Rcpp::Named("yi") = nb->yi()
                , Rcpp::Named("axon_delay") = nb->axonDelay()
                , Rcpp::Named("id") = nb->id()
                , Rcpp::Named("colSize") = nb->colSize()
                , Rcpp::Named("localId") = nb->localId()
                , Rcpp::Named("synapses") = Rcpp::List::create(
                      Rcpp::Named("weights") = Rcpp::wrap(weights)
                    , Rcpp::Named("ids_pre") = Rcpp::wrap(ids_pre)
                )
            );
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
            SEXP values = list["values"];
            if(Rf_isMatrix(values)) {
                Rcpp::NumericMatrix m(values); 
                ts->dim.size = m.nrow();
                ts->data.resize(ts->dim.size);
                for(size_t i=0; i<m.nrow(); ++i) {                    
                    for(size_t j=0; j<m.ncol(); ++j) {
                        ts->data[i].values.push_back(m(i,j));
                    }
                }

            } else {
                ts->dim.size = 1;
                ts->data.resize(ts->dim.size);
                ts->data[0].values = Rcpp::as<vector<double>>(values);
            }
            
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
