#pragma once

#include <snnlib/protos/time_series.pb.h>
#include <google/protobuf/io/zero_copy_stream_impl.h>
#include <google/protobuf/io/coded_stream.h>

#include <snnlib/core.h>

using namespace google::protobuf::io;

enum ESerializableClass { ENeuronStat, ESpikesList, ELabeledTimeSeries, ELabeledTimeSeriesList, EAdExNeuronStat };
static const char* ESerializableClass_str[] = { "NeuronStat", "SpikesList", "LabeledTimeSeries", "LabeledTimeSeriesList", "AdExNeuronStat" };


#include <snnlib/base.h>

class Serializable : public Printable {
// To make inheritance from that class need to do methods:
// LabeledTimeSeriesList(const LabeledTimeSeriesList &l) : Serializable(ELabeledTimeSeriesList), ts(l.ts) {
//     copyFrom(l);
// }
// virtual Protos::LabeledTimeSeriesList* serialize() {
//       Protos::LabeledTimeSeriesList *l = getNew();
//       for(auto it=ts.begin(); it != ts.end(); ++it) {
//           Protos::LabeledTimeSeries* lts = l->add_list();
//           *lts = *it->serialize();
//       }
//       return l;
//   }

//   virtual Protos::LabeledTimeSeriesList* getNew(google::protobuf::Message* m = nullptr) {
//       return getNewSerializedMessage<Protos::LabeledTimeSeriesList>(m);
//   }
//   virtual void deserialize() {
//       Protos::LabeledTimeSeriesList * m = castSerializableType<Protos::LabeledTimeSeriesList>(serialized_message);
//   }


public:

    template <typename CT>
    CT* castSerializableType(google::protobuf::Message* mess) {
        CT* d = dynamic_cast<CT*>(mess);
        if(!d) {
            cerr << "Errors while casting deserialized message treated as " << getName() << ":\n";
            cerr << mess->DebugString();
            terminate();
        }
    }

    Serializable(const Serializable &another) {
        copyFrom(another);
    }

    void copyFrom(const Serializable &another) {
        name = another.name;
        if(another.serialized_message) {
            serialized_message = getNew(another.serialized_message);
        }
    }

    Serializable(ESerializableClass ename) {
        name = string(ESerializableClass_str[ename]);
        serialized_message = nullptr;
    }
    virtual ::google::protobuf::Message* serialize() = 0;
    virtual void deserialize() = 0;
    virtual ::google::protobuf::Message* getNew(google::protobuf::Message* m = nullptr) = 0;


    template <typename T>
    T* getNewSerializedMessage(google::protobuf::Message* m = nullptr) {
        T *el;
        if(m) {
            el = new T(*castSerializableType<T>(m));
        } else {
            el = new T;
        }
        serialized_message = el;
        return el;
    }

    void clean() {
        if(serialized_message) {
            delete serialized_message;
            serialized_message = nullptr;
        }
    }

    ~Serializable() {
        clean();
    }

    const string& getName() {
        return name;
    }

protected:
    string name;
    ::google::protobuf::Message *serialized_message;
};




class SerializableFactory {
public:
    static SerializableFactory& inst();
    Serializable* create(const string &name);
private:
    SerializableFactory() {}
    ~SerializableFactory() {
        for(auto it=objects.begin(); it != objects.end(); ++it) {
            delete *it;
        }
        objects.clear();
    }

    vector<Serializable*> objects;
};


Protos::LabeledTimeSeries doubleVectorToLabeledTimeSeries(string label, const vector<double> &data);


