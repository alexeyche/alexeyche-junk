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

	inline T* ptr() {
		return _ptr;
	}
	inline T& ref() {
		return *_ptr;
	}
	void set(T *ptr_to_set) {
		if(_ptr) {
			throw dnnException()<< "InterfacedPtr is already set\n";
		}
		_ptr = ptr_to_set;
		_ptr->provideInterface(i);
	}
	
	bool isSet() {
		return _ptr ? true : false;
	}

	typename T::interface& ifc() {
		return i;
	}

private:
	T *_ptr;
	typename T::interface i;
};



}
