#pragma once

#include <cstring>

#include <dnn/base/base.h>
#include <typeinfo>
#include <typeindex>

namespace NDnn {

	class TAny {
	public:
		TAny()
			: Data(nullptr)
			, Size(0)
			, TypeIndex(std::type_index(typeid(void)))
		{
		}

		~TAny() {
			Clean();
		}

		TAny(const TAny& other) 
			: TypeIndex(other.TypeIndex)
		{
			(*this) = other;
		}

		TAny& operator=(const TAny& other) {
			if (this != &other) {
				Data = reinterpret_cast<void*>(new char[other.Size]);
				std::memcpy(Data, other.Data, other.Size);
				Size = other.Size;
			}
			return *this;
		}

		template <typename T>
		T GetValue() const;

		template <typename T>
		void SetValue(T v);


		void Clean() {
			if (Data) {
				delete []reinterpret_cast <char *>(Data);
			}
		}

		template <typename T>
		bool IsTypeOf() const {
			return std::type_index(typeid(T)) == TypeIndex;
		}

	private:
		template <typename T>
		T Getter() const {
			ENSURE(Data, "Data is not set");
			ENSURE(sizeof(T) == Size, "Failed to get value with " << Size << " bytes");
			ENSURE(IsTypeOf<T>(), "Type index of set value are not equivalent");
			T v = *static_cast<T*>(Data);
			return v;
		}

		template <typename T>
		void Setter(T v) {
			Clean();
			Data = reinterpret_cast<void*>(new char[sizeof(T)]);
			*reinterpret_cast<T*>(Data) = v;
			TypeIndex = std::type_index(typeid(v));
			Size = sizeof(T);
		}


		ui32 Size;
		void* Data;
		std::type_index TypeIndex;
	};

} // namespace NDnn


