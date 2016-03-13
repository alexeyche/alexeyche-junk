#pragma once

#include <dnn/util/log/log.h>

#include <typeinfo>


namespace NDnn {

	template <typename T>
	class TPtr {
	public:
		TPtr(T *ptr): Ptr(ptr) {
		}


		TPtr() : Ptr(nullptr) {
		}

		T* Get() {
			assert(Ptr);
			return Ptr;
		}
		const T* Get() const {
			assert(Ptr);
			return Ptr;
		}
		T& Ref() {
			assert(Ptr);
			return *Ptr;
		}

		const T& Ref() const {
			assert(Ptr);
			return *Ptr;
		}
		
		void Set(T *ptr) {
			Ptr = ptr;
		}
		
		void Set(T &ptr_to_set) {
			Ptr = &ptr_to_set;
		}

		const T* operator -> () const {
			return Get();
		}

		T* operator -> () {
			return Get();
		}

		const T& operator * () const {
			return Ref();
		}
		
		bool IsSet() const {
			return Ptr ? true : false;
		}

		explicit operator bool() {
			return IsSet();
		}

		template<typename NT>
		TPtr<NT> As() {
			if(!IsSet()) return TPtr<NT>();
			NT *t = dynamic_cast<NT*>(Ptr);
			if(!t) return TPtr<NT>();
			return t;
		}

	private:
		T* Ptr;
	};

} // namespace NDnn