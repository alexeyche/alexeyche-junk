#pragma once


#include <dnn/contrib/pbjson/pbjson.hpp>
#include <dnn/contrib/rapidjson/stringbuffer.h>
#include <dnn/contrib/rapidjson/prettywriter.h>
#include <dnn/contrib/rapidjson/error/en.h>

#include <dnn/util/distributions.h>


#include <google/protobuf/message.h>

namespace dnn {

class Json {
private:
	static bool __getDoubleVal(const Value &v, const string name, double &d) {
		const Value &f_v = getVal(v, name);
		if (f_v.IsDouble()) {
			d = f_v.GetDouble();
			return true;
		}
		if(f_v.IsUint()) {
			d = f_v.GetUint();
			return true;
		}
		if (f_v.IsString()) {
			uptr<Distribution<double>> distr = parseDistribution<double>(f_v.GetString());
			d = distr->getSample();
			return true;
		}
		return false;
	}
	static bool __getStringVal(const Value &v, const string name, string &d) {
		const Value &f_v = getVal(v, name);
		if (f_v.IsString()) {
			d = f_v.GetString();
			return true;
		}
		return false;
	}
	static bool __getBoolVal(const Value &v, const string name, bool &d) {
		const Value &f_v = getVal(v, name);
		if (f_v.IsBool()) {
			d = f_v.GetBool();
			return true;
		}
		return false;
	}
public:
	static const Value& getVal(const Value &v, const string name) {
		auto m = v.FindMember(name.c_str());
		if (m == v.MemberEnd()) {
			throw dnnException()<< "Erros while finding field: " << name << "\n";
		}
		return m->value;
	}

	static bool checkVal(const Value &v, const string name) {
		return v.FindMember(name.c_str()) != v.MemberEnd();
	}

	static double getDoubleVal(const Value &v, const string name) {
		double d;
		if (__getDoubleVal(v, name, d)) {
			return d;
		}
		throw dnnException()<< name << " expected as double\n";
	}


	static double getDoubleValDef(const Value &v, const string name, double def) {
		if (!checkVal(v, name)) return def;
		double d;
		if (__getDoubleVal(v, name, d)) {
			return d;
		}
		return def;
	}

	static string getStringVal(const Value &v, const string name) {
		string s;
		if (__getStringVal(v, name, s)) {
			return s;
		}
		throw dnnException()<< name << " expected as string\n";
	}
	static bool getBoolVal(const Value &v, const string name) {
		bool s;
		if (__getBoolVal(v, name, s)) {
			return s;
		}
		throw dnnException()<< name << " expected as bool\n";
	}

	static string getStringValDef(const Value &v, const string name, string def) {
		if (!checkVal(v, name)) return def;
		string s;
		if (__getStringVal(v, name, s)) {
			return s;
		}
		return def;
	}

	static size_t getUintVal(const Value &v, const string name) {
		const Value &f_v = getVal(v, name);
		if (f_v.IsUint()) {
			return f_v.GetUint();
		}
		throw dnnException()<< name << " expected as uint\n";
	}
	static size_t getIntVal(const Value &v, const string name) {
		const Value &f_v = getVal(v, name);
		if (f_v.IsInt()) {
			return f_v.GetInt();
		}
		throw dnnException()<< name << " expected as int\n";
	}
	
	static Value getArray(const Value &v, const string name) {
		Value f_v;
		const Value &f_src = getVal(v, name);		
		f_v.CopyFrom(f_src, d.GetAllocator());
		if (f_v.IsArray()) {
			return f_v;	
		}
		Value f_v_arr(kArrayType);
		f_v_arr.PushBack(f_v, d.GetAllocator());
		return f_v_arr;
	}

	static vector<size_t> getUintVector(const Value &v, const string name) {
		vector<size_t> ret;
		const Value &f_v = getArray(v, name);
		for (SizeType i = 0; i < f_v.Size(); i++) {
			if (!f_v[i].IsUint()) {
				throw dnnException()<< "Element " << i << " of " << name << " is not an uint type\n";
			}
			ret.push_back(f_v[i].GetUint());
		}
		return ret;
	}
	static string stringify(const Value &v) {
		StringBuffer buffer;
		PrettyWriter<StringBuffer> writer(buffer);
		v.Accept(writer);
		return string(buffer.GetString());
	}
	static Document* parseProtobuf(google::protobuf::Message* mess) {
		Document *document = new Document;
		rapidjson::Value* v = pbjson::pb2jsonobject(mess, document->GetAllocator());
		string str = Json::stringify(*v); // hack, because of bad library pbjson

		document->Parse(str.c_str());
		return document;
	}

	static void getRidOfComments(string &inp_s) {
		string acc;
		vector<string> lines = split(inp_s, '\n');
		for (auto s : lines) {
			{
				vector<string> subs = splitBySubstr(s, "//");
				if (subs.size() > 0) s = subs[0];
			}
			{
				vector<string> subs = splitBySubstr(s, "#");
				if (subs.size() > 0) s = subs[0];
			}
			acc += s + string("\n");
		}
		inp_s = acc;
	}
	static Document parseStringC(const string &p) {
		Document document;

		document.Parse(p.c_str());		

		if (document.HasParseError()) {
			vector<string> spl = split(p, '\n');
			int offset = document.GetErrorOffset();
			size_t line_num = 0;
			while (line_num < spl.size()) {
				cout << line_num + 1 << ": " <<  spl[line_num]  << "\n";
				if ( (offset - (int)spl[line_num].size()) < 0 ) { cout << " == somewhere in that structure an error\n"; break; }
				offset -= spl[line_num].size();
				line_num++;
			}
			throw dnnException() << "Parse JSON error:\n" 
							     << GetParseError_En(document.GetParseError()) << line_num + 1 << ":" << offset << "\n";
		}
		return document;
	}
	static Document parseString(string &p) {
		getRidOfComments(p);
		return parseStringC(p);
	}
	static void JsonToProtobuf(const Value &v, ProtoMessage m) {
		Document d;
		Value cv;
		cv.CopyFrom(v, d.GetAllocator());
		for (Value::MemberIterator itr = cv.MemberBegin(); itr != cv.MemberEnd(); ++itr) {
	        const google::protobuf::Descriptor* descriptor = m->GetDescriptor();
	        const google::protobuf::FieldDescriptor* field_descr = descriptor->FindFieldByName(itr->name.GetString());
	        
	        if(!field_descr) {
	        	continue;
	        }
	        if( (field_descr->type() == google::protobuf::FieldDescriptor::TYPE_DOUBLE) && (itr->value.IsString()) ) {
				uptr<Distribution<double>> distr = parseDistribution<double>(itr->value.GetString());
				itr->value = Value(distr->getSample());
			}
		}
		string err;
		pbjson::jsonobject2pb(&cv, m, err);
		if (!err.empty()) {
			throw dnnException()<< "Found errors while converting json to protobuf:\n";
			throw dnnException()<< err << "\n";
		}
	}
	static Document d;
};

}