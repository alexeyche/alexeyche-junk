#pragma once


#include <snnlib/protos/model.pb.h>
#include <snnlib/serialize/serialize.h>

class Neuron;

class NeuronModel : public Serializable {
protected:
    NeuronModel() : Serializable(ENeuronModel) { }
    friend class SerializableFactory;
public:
    NeuronModel(Neuron *_n) : Serializable(ENeuronModel), n(_n) {}
    
    NeuronModel(const NeuronModel &another) : Serializable(ENeuronModel) {
        copyFrom(another);
    }
    virtual Protos::NeuronModel *serialize();
    virtual void deserialize();

    virtual Protos::NeuronModel* getNew(google::protobuf::Message* m = nullptr) {
        return getNewSerializedMessage<Protos::NeuronModel>(m);
    }
    void print(std::ostream& str) const {
        str << "NeuronModel stat\n";
    }

    Neuron *n;
};

