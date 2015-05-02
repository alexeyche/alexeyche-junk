#pragma once

#include <dnn/core.h>
#include <dnn/io/serialize.h>

namespace dnn {

class ClassName;
class SerializableBase;
class SpikeNeuronBase;
class ActFunctionBase;
class TimeSeries;


class Factory {
public:
    typedef map<string, SerializableBase* (*)()> entity_map_type;
    typedef map<string, ProtoMessage (*)()> proto_map_type;
    typedef multimap<string, size_t>::iterator object_iter;

    template<typename INST> static SerializableBase* createInstance() { return new INST; }
    template<typename INST> static ProtoMessage createProtoInstance() { return new INST; }

    Factory();
    ~Factory();


    template<typename T>
    static void registerType(const string type) {
        typemap[type] = &createInstance<T>;
        if (T::hasProto) {
            prototypemap[type] = &createProtoInstance<typename T::ProtoType>;
        }
    }
    bool isProtoType(const string name) {
        return prototypemap.find(name) != prototypemap.end();
    }

    SerializableBase* createObject(string name);
    ProtoMessage createProto(string name);
    
    void registrationOff() {
        registration_is_on = false;
    }
    void registrationOn() {
        registration_is_on = true;
    }
    static Factory& inst();

    pair<object_iter, object_iter> getObjectsSlice(const string& name);
    
    SerializableBase* getObject(object_iter &it) {
        return objects[it->second];
    }
    template <typename T> 
    T* createObject(string name) {
        SerializableBase* b = createObject(name);
        T *p = dynamic_cast<T*>(b);
        if (!p) {
            throw dnnException()<< "Error to cast create object to its base" << "\n";
        }
        return p;
    }
    SerializableBase* getCachedObject(const string& filename);
private:
    bool registration_is_on;
    static entity_map_type typemap;
    static proto_map_type prototypemap;
    multimap<string, size_t> objects_map;

    vector<SerializableBase*> objects;
    vector<ProtoMessage> proto_objects;
    map<string, SerializableBase*> cache_map;
};


}
