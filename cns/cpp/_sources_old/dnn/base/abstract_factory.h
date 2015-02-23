#pragma once

#include <dnn/core.h>

namespace dnn {

class Object;

template <typename Base, typename Constructor>
class AbstractFactory {
public:
    typedef map<string, Constructor> entity_map_type;
    template<typename INST> static Base* createInstance() { return new INST; }
    AbstractFactory();
    ~AbstractFactory();


    template<typename T>
    static void registerType(const string type) {
        typemap[type] = &createInstance<T>;
    }

private:
    static entity_map_type typemap;
    vector<Base*> objects;
};



}
