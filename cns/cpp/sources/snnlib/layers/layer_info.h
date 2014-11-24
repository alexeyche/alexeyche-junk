#pragma once


#include <snnlib/serialize/serialize.h>
#include <snnlib/protos/model.pb.h>

class Layer;

class LayerInfo : public Serializable {
protected:
    LayerInfo() : Serializable(ELayerInfo) { }
    friend class SerializableFactory;
public:
    LayerInfo(Layer *_l) : Serializable(ELayerInfo), l(_l) {}
    
    LayerInfo(const LayerInfo &another) : Serializable(ELayerInfo) {
        copyFrom(another);
    }
    virtual Protos::LayerInfo *serialize();

    virtual void deserialize() {
        Protos::LayerInfo *m = castSerializableType<Protos::LayerInfo>(serialized_message);
        
    }
    virtual Protos::LayerInfo* getNew(google::protobuf::Message* m = nullptr) {
        return getNewSerializedMessage<Protos::LayerInfo>(m);
    }
    void print(std::ostream& str) const {
        str << "LayerInfo stat\n";
    }
    const Layer *l;
};


