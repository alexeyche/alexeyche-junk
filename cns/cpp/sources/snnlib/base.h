#pragma once

#include <snnlib/core.h>

#include <snnlib/util/fast_delegate.h>
using namespace fastdelegate;

typedef FastDelegate1<const double&> neuronInputDelegate;
typedef FastDelegate0<> neuronStateDelegate;

struct RunTimeDelegates {
    vector<neuronInputDelegate> input_dg;
    vector<neuronStateDelegate> state_dg;
};

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
    int sim;
    void print(std::ostream& str) const {
        str << "SynSpike(at " << t << " from " << n_id << " in synapse " << syn_id << " sim: "  << sim << ")";
    }
};

