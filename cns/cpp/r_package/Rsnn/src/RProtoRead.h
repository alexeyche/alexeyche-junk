#ifndef RPROTO_READ_H
#define RPROTO_READ_H

#define STRICT_R_HEADERS
#include <Rcpp.h>

#include "common.h"

#include <snnlib/serialize/proto_rw.h>
#include <snnlib/layers/adex_neuron.h>

class RProto {
public:
    RProto(std::string _protofile) : protofile(_protofile) {
    }
    Rcpp::List read() {
        if(values.size() == 0) {
            ProtoRw rw(protofile, ProtoRw::Read);
            Serializable *o = rw.readAny();
            if(!o) {
                ERR("Can't read protofile " << protofile << "\n");
            }
            
            vector<Serializable*> v;
            v.push_back(o);
            
            //while(true) {
            //    Serializable *o = rw.readAny();
            //    if(!o) break;
            //    cout << "reading " << o->getName() << "\n";
            //    v.push_back(o);
            //}
            if(v.size() == 1) {
                cout << v[0]->DebugString();
                values = convert(v[0]);
            } else {
                Rcpp::List read_values(v.size());
                for(size_t vi=0; vi<v.size(); vi++) {
                    read_values[vi] = convert(v[vi]);
                }
                values = read_values;
            }                
        }            
        return values;
    }
    void print() {
        cout << "RProto instance\n";
    }
    Rcpp::List convert(Serializable *s) {
        if(s->getName() == "SpikesList") {
            SpikesList *sl = dynamic_cast<SpikesList*>(s);
            if(!sl) { ERR("Can't cast"); }
            Rcpp::List sl_r(sl->N);
            for(size_t ni=0; ni<sl_r.size(); ni++) {
                sl_r[ni] = Rcpp::NumericVector(Rcpp::wrap(sl->sp_list[ni]));
            }
            return sl_r;
        } else 
        if(s->getName() == "NeuronStat") {
            NeuronStat *st = dynamic_cast<NeuronStat*>(s);
            if(!st) { ERR("Can't cast"); }
            Rcpp::NumericVector p(Rcpp::wrap(st->p));
            Rcpp::NumericVector u(Rcpp::wrap(st->u));
            Rcpp::List syns(Rcpp::wrap(st->syns));
            return Rcpp::List::create( Rcpp::Named("p") = p, Rcpp::Named("u") = u, Rcpp::Named("syns") = syns );
        } else 
        if(s->getName() == "AdExNeuronStat") {
            AdExNeuronStat *st = dynamic_cast<AdExNeuronStat*>(s);
            if(!st) { ERR("Can't cast"); }
            Rcpp::NumericVector p(Rcpp::wrap(st->ns->p));
            Rcpp::NumericVector u(Rcpp::wrap(st->ns->u));
            Rcpp::NumericVector a(Rcpp::wrap(st->a));
            Rcpp::List syns(Rcpp::wrap(st->ns->syns));
            return Rcpp::List::create( Rcpp::Named("p") = p, Rcpp::Named("u") = u, Rcpp::Named("syns") = syns, Rcpp::Named("a") = a );
        } else {
            ERR("Unknown serializable name: " << s->getName() << "\n");
        }
    }
    
        Rcpp::List values;
    std::string protofile;
};

#endif
