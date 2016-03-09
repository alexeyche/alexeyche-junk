#include "json.h"

#include <dnn/util/log/log.h>

#include <google/protobuf/descriptor.h>
#include <google/protobuf/message.h>

using namespace google::protobuf;


namespace NDnn {

	template <>
    NJson::Value TJsonDocument::SetValue<TString>(TString v) {
		NJson::Value s;
		s.SetString(v.c_str(), v.size(), Doc->GetAllocator());
		return s;
	}

	template <>
    NJson::Value TJsonDocument::SetValue<TJsonDocument>(TJsonDocument v) {
		NJson::Value s;
		s.CopyFrom(v.CurrentValue(), Doc->GetAllocator());
		return s;
	}

	template <>
    NJson::Value TJsonDocument::SetValue<double>(double v) {
		NJson::Value s;
		s.SetDouble(v);
		return s;
	}

	template <>
    NJson::Value TJsonDocument::SetValue<int>(int v) {
		NJson::Value s;
		s.SetInt(v);
		return s;
	}

	template <>
    NJson::Value TJsonDocument::SetValue<ui32>(ui32 v) {
		NJson::Value s;
		s.SetUint(v);
		return s;
	}

	template <>
	TString TJsonDocument::GetValue(const NJson::Value& v) const {
		return v.GetString();
	}

	template <>
	int TJsonDocument::GetValue(const NJson::Value& v) const {
		return v.GetInt();
	}

	template <>
	double TJsonDocument::GetValue(const NJson::Value& v) const {
		return v.GetDouble();
	}

	template <>
	bool TJsonDocument::GetValue(const NJson::Value& v) const {
		return v.GetBool();
	}

	template <>
	ui32 TJsonDocument::GetValue(const NJson::Value& v) const {
		return v.GetUint();
	}


	TString TJsonDocument::GetPrettyString() const {
		return PrettyString(CurrentValue());
	}

	TString TJsonDocument::GetString() const {
		NJson::StringBuffer buffer;
		NJson::Writer<NJson::StringBuffer> writer(buffer);
		CurrentValue().Accept(writer);
		return buffer.GetString();
	}

	TJsonDocument TJsonDocument::operator[](const TString& key) {
		if (!CurrentValue().IsObject()) {
			CurrentValue().SetObject();
		}
		auto ptr = CurrentValue().FindMember(key.c_str());
		if (ptr == CurrentValue().MemberEnd()) {
			NJson::Value val;
			NJson::Value keyVal(key.c_str(), key.size(), Doc->GetAllocator());
			CurrentValue().AddMember(keyVal, val, Doc->GetAllocator());
			return TJsonDocument(*this, CurrentValue().FindMember(key.c_str())->value);
		}
		return TJsonDocument(*this, ptr->value);
	}

	TString TJsonDocument::PrettyString(const NJson::Value& v) {
		NJson::StringBuffer buffer;
		NJson::PrettyWriter<NJson::StringBuffer> writer(buffer);
		v.Accept(writer);
		return buffer.GetString();
	}

	TJsonDocument TJsonDocument::FromAny(const TAny& any) {
		TJsonDocument doc;
		if (any.IsTypeOf<ui32>()) {
			doc.CurrentValue() = doc.SetValue<ui32>(any.GetValue<ui32>());
		} else
		if (any.IsTypeOf<TString>()) {
			doc.CurrentValue() = doc.SetValue<TString>(any.GetValue<TString>());
		} else
		if (any.IsTypeOf<double>()) {
			doc.CurrentValue() = doc.SetValue<double>(any.GetValue<double>());
		} else
		if (any.IsTypeOf<int>()) {
			doc.CurrentValue() = doc.SetValue<int>(any.GetValue<int>());
		} else {
			throw TDnnException() << "Can't find realization for type setter from any";
		}
		return doc;
	}

	TAny TJsonDocument::ToAny() const {
		TAny any;
		if (CurrentValue().IsUint()) {
			any.SetValue<ui32>(GetValue<ui32>(CurrentValue()));
		} else
		if (CurrentValue().IsString()) {
			any.SetValue<TString>(GetValue<TString>(CurrentValue()));
		} else
		if (CurrentValue().IsDouble()) {
			any.SetValue<double>(GetValue<double>(CurrentValue()));
		} else
		if (CurrentValue().IsInt()) {
			any.SetValue<int>(GetValue<int>(CurrentValue()));
		} else {
			throw TDnnException() << "Can't find realization for type getter from json to any for type " << CurrentValue().GetType() << " for value " << PrettyString(CurrentValue());
		}
		return any;
	}

	const TJsonDocument TJsonDocument::operator[](const TString& key) const {
		auto ptr = CurrentValue().FindMember(key.c_str());
		ENSURE(ptr != CurrentValue().MemberEnd(), "Failed to find " << key);
		return TJsonDocument(*this, ptr->value);
	}

	bool TJsonDocument::Has(const TString& key) const {
		return CurrentValue().FindMember(key.c_str()) != CurrentValue().MemberEnd();
	}

	std::ostream& operator<<(std::ostream& stream, const TJsonDocument& doc) {
		stream << doc.GetPrettyString();
		return stream;
	}

	TVector<TString> TJsonDocument::GetKeys() const {
		TVector<TString> keys;
		for (auto ptr = CurrentValue().MemberBegin(); ptr != CurrentValue().MemberEnd(); ++ptr) {
			keys.push_back(ptr->name.GetString());
		}
		return keys;
	}

} // namespace NDnn