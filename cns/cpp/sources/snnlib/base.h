#pragma once

#include <snnlib/core.h>

#include <snnlib/util/fast_delegate.h>
using namespace fastdelegate;

class SynSpike;

typedef FastDelegate0<> stateDelegate;
typedef FastDelegate1<const double&, double> funDelegate;
typedef FastDelegate1<const SynSpike *> propSynSpikeDelegate;
typedef FastDelegate1<const double&> attachDelegate;

class Obj {
public:
    virtual ~Obj() {
    }

};

class Printable: public Obj {
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
template <typename T>
void print_deque(deque<T> v, ostream &str, string sep) {
    std::copy(v.cbegin(), v.cend(), ostream_iterator<T>(str, sep.c_str()));
    str << "\n";
}


class Entity : public Printable {
};

typedef map<string, unique_ptr<Entity> > entity_map;

class SynSpike: public Printable {
public:
    double t;
	size_t n_id;
	size_t syn_id;
    // int sim;

    void print(std::ostream& str) const {
        str << "SynSpike(at " << t << " from " << n_id << " in synapse " << syn_id << ")"; // " sim: " << sim << ")";
    }
};

template <typename T>
const T* castType(const Obj* o) {
    const T *cast = dynamic_cast<const T*>(o);
    if(!cast) {
        cerr << "Instance was created with constants different type\n";
        terminate();
    }
    return cast;
}




