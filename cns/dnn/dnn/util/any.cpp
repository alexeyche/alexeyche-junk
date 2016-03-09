#include "any.h"

namespace NDnn {



	template <>
	ui32 TAny::GetValue() const {
		return Getter<ui32>();
	}

	template <>
	int TAny::GetValue() const {
		return Getter<int>();
	}

	template <>
	double TAny::GetValue() const {
		return Getter<double>();
	}

	template <>
	TString TAny::GetValue() const {
		ENSURE(Data, "Data is not set");
		return TString(static_cast<char*>(Data), Size);
	}

	template <>
	void TAny::SetValue(ui32 v) {
		Setter<ui32>(v);
	}

	template <>
	void TAny::SetValue(int v) {
		Setter<int>(v);
	}

	template <>
	void TAny::SetValue(double v) {
		Setter<double>(v);
	}

	template <>
	void TAny::SetValue(TString v) {
		Clean();
		Data = reinterpret_cast<void*>(new char[v.size()]);
		std::memcpy(Data, v.c_str(), v.size());
		Size = v.size();
		TypeIndex = std::type_index(typeid(v));
	}


} // namespace NDnn


