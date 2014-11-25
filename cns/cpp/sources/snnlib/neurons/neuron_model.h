#pragma once


#include <snnlib/protos/model.pb.h>

class NeuronModel : public Serializable {
protected:
    NeuronModel() : Serializable(ENeuronModel) { }
    friend class SerializableFactory;
public:
    NeuronModel(Neuron *_n) : Serializable(ENeuronModel), n(_n) {}
    
    NeuronModel(const NeuronModel &another) : Serializable(ENeuronModel) {
        copyFrom(another);
    }
    virtual Protos::NeuronModel *serialize() {
        Protos::NeuronModel *stat = getNew();
        for(auto it=n->syns.begin(); it != n->syns.end(); ++it) {
            Synapse *s = *it;
            Protos::NeuronModel::Syns *syns = stat->add_syns();
            syns->set_w(s->w);
            syns->set_id_pre(s->id_pre);
            syns->set_dendrite_delay(s->dendrite_delay);
        }
        stat->set_axon_delay(n->axon_delay);
        return stat;
    }

    virtual void deserialize() {
        Protos::NeuronModel *m = castSerializableType<Protos::NeuronModel>(serialized_message);
        n->syns.clear();
        for(size_t si=0; si<m->syns_size(); si++) {
            
            n->syns
        }    
    }
    virtual Protos::NeuronModel* getNew(google::protobuf::Message* m = nullptr) {
        return getNewSerializedMessage<Protos::NeuronModel>(m);
    }
    void print(std::ostream& str) const {
        str << "NeuronModel stat\n";
    }


    Neuron *n;
};

