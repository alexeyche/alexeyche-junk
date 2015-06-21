#pragma once

namespace dnn {

template <typename T>
class Ptr {
public:
	Ptr(T *ptr_to_set) : _ptr(ptr_to_set) {		
	}

	Ptr() : _ptr(nullptr) {
	}
	inline T* ptr() {
		assert(_ptr);
		return _ptr;
	}
	inline T& ref() {
		assert(_ptr);
		return *_ptr;
	}
	void set(T *ptr_to_set) {
		_ptr = ptr_to_set;		
	}
	void set(T &ptr_to_set) {
		_ptr = &ptr_to_set;		
	}
	T& operator -> () {
		assert(_ptr);
		return ref();
	}
	
	bool isSet() {
		return _ptr ? true : false;
	}
private:
	T *_ptr;
};

}