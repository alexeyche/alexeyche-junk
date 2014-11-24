#pragma once


#include "serialize.h"

#include <snnlib/layers/adex_neuron.h>
#include <snnlib/layers/neuron.h>

class Stat : public Serializable {
public:
    Stat() : Serializable(EStat) {}


    Stat(const Stat &another) : Serializable(EStat) {
    }
    virtual Protos::AdExNeuronStat *serialize() {
        Protos::Stat *stat = getNew();
        for(auto it=a.begin(); it != a.end(); ++it) {
            stat->add_a(*it);
        }
        return stat;
    }
    virtual void deserialize() {
        Protos::AdExNeuronStat * m = castSerializableType<Protos::AdExNeuronStat>(serialized_message);
        for(size_t i=0; i<m->a_size(); i++) {
            a.push_back(m->a(i));
        }
    }
    virtual Protos::AdExNeuronStat* getNew(google::protobuf::Message* m = nullptr) {
        return getNewSerializedMessage<Protos::AdExNeuronStat>(m);
    }

    void print(std::ostream& str) const {}

    vector<double> a;
};
