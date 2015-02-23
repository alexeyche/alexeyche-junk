#pragma once


template <typename T> 
class InterfacedPtr {
public:
	InterfacedPtr(T *_ptr) : ptr(_ptr) {
		ptr->provideInterface(i);
	}

	InterfacedPtr() : ptr(nullptr) {
		T::provideDefaultInterface(i);
	}

	T* ptr() {
		return ptr;
	}
	void set(T *_ptr) {
		if(ptr) {
			cerr << "InterfacedPtr is already set\n";
			terminate();
		}
		ptr = _ptr;
		ptr->provideInterface(i);
	}

	T::interface& operator ->() {
		return i;
	}

private:
	T *ptr;
	T::interface i;
};