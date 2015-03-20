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
        Sim::run(jobs);
    }
    
    void setTimeSeries(const Rcpp::NumericVector &v) {
        Rcpp::List tsl;
        tsl["values"] = v;
        TimeSeries* ts = RProto::convertBack<TimeSeries>(tsl, "TimeSeries");
        for(auto &n: neurons) {
            if(n.ref().inputIsSet()) {
                n.ref().getInput().setTimeSeries(ts);
                duration = std::max(duration, n.ref().getInput().getDuration());
            }
        }
    }

    Rcpp::List getStat() {
        Rcpp::List out;
        for(auto &n: neurons) {
            if(n.ref().getStat().on()) {
                stringstream ss;
                ss << n.ref().name() << "_" << n.ref().id();
                out[ss.str()] = RProto::convertToList(&n.ref().getStat());
            }
        }
        return out;
    }
    Rcpp::List getSpikes() {
        if(!net.get()) {
            throw dnnException()<< "Sim network was not found. You need to build sim\n";
        }
        return RProto::convertToList(&net->spikesList());
    }
};



#endif
