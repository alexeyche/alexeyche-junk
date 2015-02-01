#pragma once

#include <iostream>

using std::cout;
using std::move;
using std::unique_ptr;
using std::vector;

void test();


class DynamicObject {
public:	
	virtual void step() = 0;
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

class LeakyIntegrateAndFire : public DynamicObject {
	void step() {
		
	}
	
	double x;
};