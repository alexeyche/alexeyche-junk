#ifndef RPROTO_READ_H
#define RPROTO_READ_H

#define STRICT_R_HEADERS
#include <Rcpp.h>

#include "common.h"

#include <snnlib/serialize/proto_rw.h>
#include <snnlib/serialize/serialize.h>
#include <snnlib/neurons/adex_neuron.h>

class RProto {
public:
    RProto(std::string _protofile) : protofile(_protofile) {
    }
    Rcpp::List read() {
        if(values.size() == 0) {
            try {
                ProtoRw rw(protofile, ProtoRw::Read);

                SerializableBase *o = rw.read();
                if(!o) {
                    ERR("Can't read protofile " << protofile << "\n");
                }
                
                SerialPack v({o});
                
                while(true) {
                    SerializableBase *o = rw.read();
                    if(!o) break;
                    v.push_back(o);
                }
                if(v.size() == 1) {
                    values = convert(v[0]);
                } else {
                    for(size_t vi=0; vi<v.size(); vi++) {
                        stringstream ss;
                        ss << v[vi]->getName() << "_" << vi;
                        values[ss.str()] = convert(v[vi]);
                    }
                }                
            } catch(...) {
                ERR("Can't read protofile " << protofile << "\n");
            }
        }            
        return values;
    }
    void print() {
        cout << "RProto instance. run instance$read() method to read protobuf\n";
    }
    Rcpp::List convert(SerializableBase *s) {
        Rcpp::List out;
        if(s->getName() == "SpikesList") {
            SpikesList *sl = dynamic_cast<SpikesList*>(s);
            if(!sl) { ERR("Can't cast"); }
            for(size_t ni=0; ni<sl->N; ni++) {
                stringstream ss;
                ss << ni;
                out[ss.str()] = Rcpp::NumericVector(Rcpp::wrap(sl->sp_list[ni]));
            }
        } else 
        if((s->getName() == "NeuronStat")||(s->getName() == "AdExNeuronStat")) {
            NeuronStat *st = dynamic_cast<NeuronStat*>(s);
            if(!st) { ERR("Can't cast"); }
            out["p"] = Rcpp::wrap(st->p);
            out["u"] = Rcpp::wrap(st->u);
            out["syns"] = Rcpp::wrap(st->syns);
        } else 
        if(s->getName() == "AdExNeuronStat") {
            AdExNeuronStat *st = dynamic_cast<AdExNeuronStat*>(s);
            if(!st) { ERR("Can't cast"); }
            out["a"] = Rcpp::wrap(st->a);
        } else {
            ERR("Unknown serializable name: " << s->getName() << "\n");
        }
        return out;
    }
    
    Rcpp::List values;
    std::string protofile;
};



#endif
