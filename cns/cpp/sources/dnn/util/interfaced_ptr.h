#pragma once

namespace dnn {


template <typename T>
class InterfacedPtr {
public:
	InterfacedPtr(T *ptr_to_set) : _ptr(ptr_to_set) {
		_ptr->provideInterface(i);
	}

	InterfacedPtr() : _ptr(nullptr) {
		T::provideDefaultInterface(i);
	}

	T* ptr() {
		return _ptr;
	}
	T& ref() {
		return *_ptr;
	}
	void set(T *ptr_to_set) {
		if(_ptr) {
			cerr << "InterfacedPtr is already set\n";
			terminate();
		}
		_ptr = ptr_to_set;
		_ptr->provideInterface(i);
	}
	
	bool isSet() {
		return _ptr ? true : false;
	}

	typename T::interface& getInterface() {
		return i;
	}

private:
	T *_ptr;
	typename T::interface i;
};



}
