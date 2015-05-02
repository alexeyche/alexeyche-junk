#ifndef RSIM_H
#define RSIM_H


#include <dnn/sim/sim.h>

#undef PI
#define STRICT_R_HEADERS
#include <Rcpp.h>

#include "RConstants.h"
#include "RProto.h"

class RSim : public dnn::Sim {
public:
    RSim(RConstants *rc) try : Sim(*rc)  {
        global_neuron_index = 0;
        Sim::build();
    } catch (std::exception &e) {
        ERR("\nCan't build Sim: " << e.what() << "\n");
    }
    ~RSim() { }
    
    void print() {
        cout << *this;
    }
    
    void run(size_t jobs=1) {
        try {
            Sim::run(jobs);
        } catch(std::exception &e) {
            ERR("Sim run failed: " << e.what() << "\n" );
        }
    }
    
    void setTimeSeries(const Rcpp::NumericVector &v, const string &obj_name) {
        Rcpp::List tsl;
        tsl["values"] = v;
        TimeSeries* ts = RProto::convertBack<TimeSeries>(tsl, "TimeSeries");
        auto slice = Factory::inst().getObjectsSlice(obj_name);
        for(auto it=slice.first; it != slice.second; ++it) {
            Factory::inst().getObject(it)->setAsInput(
                ts
            );
        }        
        for(auto &n: neurons) {
            duration = std::max(duration, n.ref().getSimDuration());
        }
    }

    void setInputSpikes(const Rcpp::List &l, const string &obj_name) {
        Rcpp::List sl;
        sl["values"] = l;
        SpikesList* sp_l = RProto::convertBack<SpikesList>(sl, "SpikesList");
        auto slice = Factory::inst().getObjectsSlice(obj_name);
        for(auto it=slice.first; it != slice.second; ++it) {
            Factory::inst().getObject(it)->setAsInput(
                sp_l
            );
        }        
        for(auto &n: neurons) {
            duration = std::max(duration, n.ref().getSimDuration());
        }
    }
    Rcpp::List getStat() {
        Rcpp::List out;
        for(auto &n: neurons) {
            if(n.ref().getStat().on()) {
                stringstream ss;
                ss << n.ref().name() << "_" << n.ref().id();
                Statistics st = n.ref().getStat();
                out[ss.str()] = RProto::convertToList(&st);
            }
        }
        return out;
    }
    Rcpp::List getModel() {
        Rcpp::NumericMatrix w(neurons.size(), neurons.size());
        
        for(auto &n: neurons) {
            for(auto &syn: n.ref().getSynapses()) {
                w(syn.ref().idPre(), n.ref().id()) = syn.ref().weight();
            }
        }
        return Rcpp::List::create(
            Rcpp::Named("w") = w
        );
    }
    void saveModel(const string &fname) {
        ofstream fstr(fname);
        Stream str_out(fstr, Stream::Binary);
        serialize(str_out);
    }
    Rcpp::List getSpikes() {
        if(!net.get()) {
            throw dnnException()<< "Sim network was not found. You need to build sim\n";
        }
        return RProto::convertToList(&net->spikesList());
    }
};



#endif
