#pragma once

#include <dnn/core.h>

namespace dnn {

class ClassName;
class SerializableBase;

class Factory {
public:
    typedef map<string, SerializableBase* (*)()> entity_map_type;

    template<typename INST> static SerializableBase* createInstance() { return new INST; }

    Factory();
    ~Factory();


    template<typename T>
    static void registerType(const string type) {
        typemap[type] = &createInstance<T>;
    }

    SerializableBase* createObject(string name);

    static Factory& inst();


private:
    static entity_map_type typemap;
    vector<SerializableBase*> objects;
};


}