#pragma once


#include <dnn/contrib/pbjson/pbjson.hpp>
#include <dnn/contrib/rapidjson/stringbuffer.h>
#include <dnn/contrib/rapidjson/prettywriter.h>
#include <dnn/contrib/rapidjson/error/en.h>

#include <google/protobuf/message.h>


namespace dnn {

class Json {
public:	
	static const Value& getVal(const Value &v, const string name) {
		auto m = v.FindMember(name.c_str());
		if(m == v.MemberEnd()) {
			cerr << "Erros while finding field: " << name << "\n";
			terminate();
		}
		return m->value;
	}

	static double getDoubleVal(const Value &v, const string name) {
		const Value &f_v = getVal(v, name);
		if(f_v.IsDouble()) {
			return f_v.GetDouble();
		}
		cerr << name << " expected as double\n";
		terminate();
	}		
	static string getStringVal(const Value &v, const string name) {
		const Value &f_v = getVal(v, name);
		if(f_v.IsString()) {
			return f_v.GetString();
		}

		cerr << name << " expected as string\n";
		terminate();
	}		
	
	static string getStringValDef(const Value &v, const string name, string def) {
		const Value &f_v = getVal(v, name);
		if(f_v.IsString()) {
			return f_v.GetString();
		}		
		return def;
	}

	static size_t getUintVal(const Value &v, const string name) {
		const Value &f_v = getVal(v, name);
		if(f_v.IsUint()) {
			return f_v.GetUint();
		}
		cerr << name << " expected as uint\n";
		terminate();
	}
	static size_t getIntVal(const Value &v, const string name) {
		const Value &f_v = getVal(v, name);
		if(f_v.IsInt()) {
			return f_v.GetInt();
		}
		cerr << name << " expected as int\n";
		terminate();
	}
	static const Value& getArray(const Value &v, const string name) {
		const Value &f_v = getVal(v, name);
		if(!f_v.IsArray()) {
        	cerr << name << " must be array\n";
        	terminate();
        }
        return f_v;
	}
	static vector<size_t> getUintVector(const Value &v, const string name) {
		vector<size_t> ret;
		const Value &f_v = getArray(v, name);
		for (SizeType i = 0; i < f_v.Size(); i++) {
			if(!f_v[i].IsUint()) {
				cerr << "Element " << i << " of " << name << " is not an uint type\n";
				terminate();
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
	static Document parseString(const string p) {
		Document document;

		document.Parse(p.c_str());
		
		if(document.HasParseError()) {
			vector<string> spl = split(p, '\n');
			int offset = document.GetErrorOffset();
			size_t line_num = 0;
			while(line_num < spl.size()) {
				cout << line_num+1 << ": " <<  spl[line_num]  << "\n";
				if( (offset - (int)spl[line_num].size())<0 ) { cout << " == somewhere in that structure an error\n"; break; }
				offset -= spl[line_num].size();
				line_num++;
			}
			cerr << "Parse JSON error:\n";
			cerr << GetParseError_En(document.GetParseError()) << line_num+1 << ":" << offset << "\n";
			terminate();
		}
		return document;
	}
};

}