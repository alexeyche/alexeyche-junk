#pragma once

#include <snnlib/protos/stat.pb.h>
#include <snnlib/serialize/serialize.h>


class Neuron;

class NeuronStat : public Serializable {
protected:
    NeuronStat() : Serializable(ENeuronStat) { }
    friend class SerializableFactory;    
public:    
    NeuronStat(Neuron *n);
    

    vector<vector<double>> syns;
    vector<double> p;
    vector<double> u;
    
    void collect(Neuron *n);

    NeuronStat(const NeuronStat &another) : Serializable(ENeuronStat), syns(another.syns), p(another.p), u(another.u) {
        copyFrom(another);
    }
    virtual Protos::NeuronStat *serialize();

    virtual void deserialize() {
        cerr << "Why you need that?\n";
        terminate();
    }
    virtual Protos::NeuronStat* getNew(google::protobuf::Message* m = nullptr) {
        return getNewSerializedMessage<Protos::NeuronStat>(m);
    }
    
    void print(std::ostream& str) const {
    }
};

