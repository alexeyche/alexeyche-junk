#pragma once

#include <dnn/core.h>

namespace dnn {

class Object;

class Factory {
public:
    typedef map<string, Object*(*)()> entity_map_type;
    template<typename INST> static Object* createInstance() { return new INST; }
    Factory();
    ~Factory();

    template <typename DST>
    DST* createObject(string name) {
        Object *o = typemap[name]();
        objects.push_back(o);
        DST *od = dynamic_cast<DST*>(o);
        if(!od) {
            cerr << "Error while creating object " << name << "\n";
            terminate();
        }
        return od;
    }

    template<typename T>
    static void registerType(const string type) {
        typemap[type] = &createInstance<T>;
    }
    static Factory& inst();

private:
    static entity_map_type typemap;
    vector<Object*> objects;
};

#define REG_TYPE(TYPENAME) \
    Factory::registerType<TYPENAME>(#TYPENAME); \


}
