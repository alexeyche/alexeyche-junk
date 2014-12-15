#pragma once

#include <snnlib/protos/time_series.pb.h>
#include <google/protobuf/io/zero_copy_stream_impl.h>
#include <google/protobuf/io/coded_stream.h>

#include <snnlib/core.h>

using namespace google::protobuf::io;

enum ESerializableClass {
                            ENeuronStat,
                            ESpikesList,
                            ELabeledTimeSeries,
                            ELabeledTimeSeriesList,
                            EAdExNeuronStat,
                            ENeuron,
                            ELayerInfo,
                            ESynapse,
                            EBlankModel,
                            EOptimalStdp,
                            ESRMNeuron,
                            EAdExNeuron,
                            EConstants,
                            EMaxLikelihood,
                            ELikelihood,
                            EReward,
                            ERewardStat,
                            EMaxLikelihoodStat,
                            EInputClassification,
                            EMeanActivityHomeostasis,
                            EMinMax,
                            ESim,
                            EOptimalStdpStat,
                            EStdp,
                            EStdpStat
                        };

static const char* ESerializableClass_str[] =
                                            {
                                                "NeuronStat",
                                                "SpikesList",
                                                "LabeledTimeSeries",
                                                "LabeledTimeSeriesList",
                                                "AdExNeuronStat",
                                                "Neuron",
                                                "LayerInfo",
                                                "Synapse",
                                                "BlankModel",
                                                "OptimalStdp",
                                                "SRMNeuron",
                                                "AdExNeuron",
                                                "Constants",
                                                "MaxLikelihood",
                                                "Likelihood",
                                                "Reward",
                                                "RewardStat",
                                                "MaxLikelihoodStat",
                                                "InputClassification",
                                                "MeanActivityHomeostasis",
                                                "MinMax",
                                                "Sim",
                                                "OptimalStdpStat",
                                                "Stdp",
                                                "StdpStat"
                                            };


#include <snnlib/base.h>

typedef vector<google::protobuf::Message*> ProtoPack;

class SerializableBase : public Printable {
public:
    SerializableBase() {}
    SerializableBase(ESerializableClass ename) {
        init(ename);
    }

    void init(ESerializableClass ename) {
        name = string(ESerializableClass_str[ename]);
    }

    virtual ProtoPack serialize() = 0;
    virtual void deserialize() = 0;
    virtual ProtoPack getNew() = 0;

    const string& getName() const {
        if(name.empty()) {
            cerr << "Trying to get name from uninitialized Serializable object\n";
            terminate();
        }
        return name;
    }
    template <typename CT>
    CT* castSerializable() {
        CT* d = dynamic_cast<CT*>(this);
        if(!d) {
            cerr << "Errors while casting deserialized message treated as " << getName() << ":\n";
            terminate();
        }
        return d;
    }
protected:
    string name;
};

typedef vector<SerializableBase*> SerialPack;


template <typename T>
class Serializable : public SerializableBase {
public:
    Serializable(ESerializableClass ename) {
        init(ename);
    }
    template <typename CT>
    CT* castProtoMessage(google::protobuf::Message* mess) {
        CT* d = dynamic_cast<CT*>(mess);
        if(!d) {
            cerr << "Errors while casting deserialized message treated as " << getName() << ":\n";
            cerr << mess->DebugString();
            terminate();
        }
        return d;
    }


    Serializable(const Serializable &another) {
        copyFrom(another);
    }
    Serializable& operator=(const Serializable &another) {
        copyFrom(another);
        return *this;
    }


    template <typename D>
    D* getNewSerializedMessage(google::protobuf::Message* m = nullptr) {
        D *el;
        if(m) {
            el = new D(*castProtoMessage<D>(m));
        } else {
            el = new D;
        }
        serialized_messages.push_back(el);
        return el;
    }

    T* getNewSerializedMessage(google::protobuf::Message* m = nullptr) {
        return getNewSerializedMessage<T>(m);
    }

    void copyFrom(const Serializable &another) {
        name = another.name;
        for(size_t mi=0; mi<another.serialized_messages.size(); mi++) {
            getNewSerializedMessage(another.serialized_messages[mi]);
        }
    }



    virtual ProtoPack serialize() = 0;
    virtual void deserialize() = 0;
    virtual ProtoPack getNew() {
        return ProtoPack({ getNewMessage() });
    }

    T* getNewMessage() {
        return getNewMessage<T>();
    }

    template <typename D>
    D* getNewMessage() {
        return getNewSerializedMessage<D>();
    }

    T* getSerializedMessage(size_t i = 0) {
        return getSerializedMessage<T>(i);
    }

    template <typename D>
    D* getSerializedMessage(size_t i = 0) {
        if(i >= serialized_messages.size()) {
            cerr << "Trying to get message which haven't been serialized by " << name << "\n";
            terminate();
        }
        return castProtoMessage<D>(serialized_messages[i]);
    }


    void clean() {
        if(serialized_messages.size()>0) {
            for(auto it=serialized_messages.begin(); it != serialized_messages.end(); ++it) {
                delete *it;
            }
        }
    }

    ~Serializable() {
        clean();
    }


protected:

    ProtoPack serialized_messages;
};




Protos::LabeledTimeSeries doubleVectorToLabeledTimeSeries(string label, const vector<double> &data);


