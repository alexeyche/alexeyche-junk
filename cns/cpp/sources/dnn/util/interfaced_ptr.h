#pragma once

namespace dnn {




template <typename T>
class InterfacedPtr {
public:
	InterfacedPtr(T *ptr_to_set) : _ptr(ptr_to_set) {
		_ptr->template provideInterface<T>(i);
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
		_ptr = ptr_to_set;
		_ptr->template provideInterface<T>(i);
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
