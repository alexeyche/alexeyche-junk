#pragma once

#include <snnlib/core.h>

class Obj {
};

class Printable: Obj {
protected:
    virtual void print(std::ostream& str) const = 0;
public:    
    friend std::ostream& operator<<(std::ostream& str, Printable const& data) {
        data.print(str);
        return str;
    }
};


template <typename T>
void print_vector(vector<T> v, ostream &str, string sep) {
    std::copy(v.cbegin(), v.cend(), ostream_iterator<T>(str, sep.c_str()));
    str << "\n";
}

class Entity : public Printable {
};

typedef map<string, unique_ptr<Entity> > entity_map;

