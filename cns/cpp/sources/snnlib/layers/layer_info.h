#pragma once


#include <snnlib/serialize/serialize.h>
#include <snnlib/protos/model.pb.h>

class Layer;

class LayerInfo : public Serializable<Protos::LayerInfo> {
protected:
    LayerInfo() : Serializable<Protos::LayerInfo>(ELayerInfo) { }
    friend class Factory;
public:
    LayerInfo(Layer *_l) : Serializable<Protos::LayerInfo>(ELayerInfo), l(_l) {}

    virtual ProtoPack serialize();

    virtual void deserialize() {
        Protos::LayerInfo *m = getSerializedMessage();

    }
    void print(std::ostream& str) const {
        str << "LayerInfo stat\n";
    }
    const Layer *l;
};


