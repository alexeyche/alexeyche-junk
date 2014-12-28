#ifndef RSIM_H
#define RSIM_H


#include <snnlib/sim/sim.h>
#include <snnlib/neurons/neuron.h>

#define STRICT_R_HEADERS
#include <Rcpp.h>

#include "RConstants.h"
#include "RProto.h"

class RSim : public Sim {
public:
    RSim(RConstants *rc, size_t jobs=1) : Sim(jobs)  {
        global_neuron_index = 0;
        construct(*rc);
    }
    ~RSim() { }
    void print() {
        cout << *this;
    }
    void setInputSpikesList(const Rcpp::List &l) {
        SerializableBase *sl_n = Factory::inst().createSerializable("LabeledSpikesList");
        LabeledSpikesList *sl = dynamic_cast<LabeledSpikesList*>(sl_n);
        if(!sl) {
            ERR("Undefined behaviour\n");
        }
        ProtoPack p = RProto::listToProtobuf(l, "LabeledSpikesList");
        if(p.size() != 1) {
            ERR("Undefined behaviour\n");
        }
        sl->deserializeFromAllocated(p[0]);
        Sim::setInputSpikesList(*sl);
    }
    void run() {
        Sim::run();
    }
    Rcpp::List getSpikes() {
        return RProto::convertToList(&net.spikes_list);
    }
    Rcpp::List getModel() {
        return RProto::readModel(this);
    }
    Rcpp::List getStat() {
        Rcpp::List values;
        SerialPack v = Sim::saveStat();
        for(size_t vi=0; vi<v.size(); vi++) {
            stringstream ss;
            ss << v[vi]->getName() << "_" << vi;
            values[ss.str()] = RProto::convertToList(v[vi]);
        }
        return values;
    }
    void collectStat() {
        Sim::turnOnStatCollect();
        cout << "Statistics will be collected\n";
    }
};



#endif
