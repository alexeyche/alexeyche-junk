#pragma once

#include <iostream>

#include <dnn/base/base.h>
#include <dnn/util/optional.h>
#include <dnn/util/any.h>

#include <dnn/contrib/rapidjson/writer.h>
#include <dnn/contrib/rapidjson/document.h>
#include <dnn/contrib/rapidjson/prettywriter.h>

namespace NJson = rapidjson;

namespace NDnn {

	class TJsonDocument {
	// 	friend class TJsonDocumentWrap;
	public:

		static TJsonDocument FromAny(const TAny& any); 

		template <typename T>
		static TJsonDocument Array(T array) {
			TJsonDocument doc;
			doc.Doc->SetArray();
			doc.FormArray(array);
			return doc;
		}


		template <typename T>
		static TJsonDocument Array(const std::initializer_list<T>& array) {
			return Array(std::vector<T>(array));
		}

		static TJsonDocument Array() {
			TJsonDocument doc;
			doc.Doc->SetArray();
			return doc;
		}

		static TJsonDocument Object() {
			TJsonDocument doc;
			doc.Doc->SetObject();
			return doc;
		}

		static TString PrettyString(const NJson::Value& v);

		TJsonDocument() {
			Doc = std::make_shared<NJson::Document>();
		}

		TJsonDocument(const TString& json) {
			Doc = std::make_shared<NJson::Document>();
			Doc->Parse(json.c_str());
		}

		TJsonDocument(const TJsonDocument& anotherDoc, NJson::Value& leaf) {
			Doc = anotherDoc.Doc;
			Leaf.emplace(leaf);
		}

		TJsonDocument(const TJsonDocument& anotherDoc, const NJson::Value& cleaf) {
			Doc = anotherDoc.Doc;
			CLeaf.emplace(cleaf);
		}

		template <typename T>
		NJson::Value SetValue(T v);

		template <typename T>
		T GetValue(const NJson::Value& v) const;

		template <typename T>
		T GetValue() const {
			return GetValue<T>(CurrentValue());
		}

		template <typename T>
		TJsonDocument& PushBack(T v) {
			if (!CurrentValue().IsArray()) {
				CurrentValue().SetArray();
			}
			CurrentValue().PushBack(SetValue(v), Doc->GetAllocator());
			return *this;
		}

		template <typename T>
		TJsonDocument& FormArray(T arr) {
			if (!CurrentValue().IsArray()) {
				CurrentValue().SetArray();
			}
			for (const auto& v: arr) {
				PushBack(v);
			}
			return *this;
		}

		TString GetPrettyString() const;

		TString GetString() const;

		TJsonDocument operator[](const TString& key);

		const TJsonDocument operator[](const TString& key) const;

		bool Has(const TString& key) const;

		TJsonDocument& operator=(const TJsonDocument& anotherVal) {
			CurrentValue().CopyFrom(*anotherVal.Doc, Doc->GetAllocator());
			return *this;
		}

		template <typename T>
		TJsonDocument& operator=(const T& anotherVal) {
			CurrentValue().CopyFrom(SetValue(anotherVal), Doc->GetAllocator());
			return *this;
		}

		template <typename T>
		TJsonDocument& operator=(const TVector<T>& anotherVal) {
			FormArray(anotherVal);
			return *this;
		}

        friend std::ostream& operator<<(std::ostream& stream, const TJsonDocument& doc);

        TVector<TString> GetKeys() const;

        TAny ToAny() const;
	private:
		NJson::Value& CurrentValue() {
			if (Leaf) {
				return *Leaf;
			}
			return *Doc;
		}

		const NJson::Value& CurrentValue() const {
			if (CLeaf) {
				return *CLeaf;
			}
			return *Doc;
		}

		SPtr<NJson::Document> Doc;

		TOptional<NJson::Value&> Leaf;
		TOptional<const NJson::Value&> CLeaf;
	};

} // namespace NDnn 