#pragma once

#include <iostream>

using std::cout;
using std::move;
using std::unique_ptr;
using std::vector;

void test();


template <typename Constants>
class DynamicObject {
public:
    DynamicObject(Constants &_c) : c(_c) {}
	void step() {

    }

private:
    Constants &c;
};


class System : public DynamicObject {
public:
	void add(unique_ptr<DynamicObject> o) {
		dyn_objects.push_back( move(o) );
	}

	void step() {
		for(auto it=dyn_objects.begin(); it != dyn_objects.end(); ++it) {
			it->step();
		}
	}

	vector<unique_ptr<DynamicObject>> dyn_objects;
};


class OneStateDynamicObject: public DynamicObject {
public:

    vector<const double&> shared_states;
    double state;
};

class LIF : public OneStateDynamicObject {
    void step() {

    }
};

