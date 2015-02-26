#pragma once

#include <dnn/contrib/rapidjson/document.h>
#include <dnn/contrib/rapidjson/stringbuffer.h>
#include <dnn/contrib/rapidjson/prettywriter.h>
#include <dnn/contrib/rapidjson/filestream.h>
#include <dnn/contrib/rapidjson/error/en.h>


using namespace rapidjson;

static const char* kTypeNames[] = 
    { "Null", "False", "True", "Object", "Array", "String", "Number" };

class Constants {
public:
	Constants(const string& fname) {
		std::ifstream ifs(fname);
		std::string const_json((std::istreambuf_iterator<char>(ifs)),
                 std::istreambuf_iterator<char>());
		
		Document document;

		
		document.Parse(const_json.c_str());
		if(document.HasParseError()) {
			cerr << "Parse JSON error:\n";
			cerr << GetParseError_En(document.GetParseError()) <<  document.GetErrorOffset() << "\n";
			terminate();
		}
		const Value &neurons = document["neurons"];
		
		for (Value::ConstMemberIterator itr = neurons.MemberBegin(); itr != neurons.MemberEnd(); ++itr) {
		    printf("Type of member %s is %s\n", itr->name.GetString(), kTypeNames[itr->value.GetType()]);
		}

		// string str;
		// StringBuffer buffer;
		// PrettyWriter<StringBuffer> writer(buffer);
		// a.Accept(writer);
		// str.append(buffer.GetString(), buffer.GetSize());
		// cout << str;
	}
};

