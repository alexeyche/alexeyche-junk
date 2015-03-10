#include "stream.h"

#include <dnn/io/serialize.h>
#include <dnn/protos/base.pb.h>

namespace dnn {

// Stream& Stream::operator << (SerializableBase &v) {
//     v.processStream(*this);
//     return *this;
// }




void Stream::writeObject(SerializableBase *b) {
	if(!isOutput()) {
		cerr << "Stream isn't open in output mode. Need output stream\n";
		terminate();
	}

	vector<ProtoMessage> messages = b->getSerialized();

	if(getRepr() == Text) {
		vector<string> buff;
		Document d;
		Value o(kObjectType);
		Value sub_o(kObjectType);
		for(auto &m: messages) {
			cout << "============\n";
			cout << m->DebugString() << "\n";
			cout << "==============\n";
			Document *sub_d = Json::parseProtobuf(m);
			cout << "============\n";
			cout << Json::stringify(*sub_d) << "\n";
			cout << "==============\n";
			buff.push_back(m->GetTypeName());
			sub_o.AddMember(StringRef(buff.back().c_str()), *sub_d, d.GetAllocator());
		}
		string temps = b->name();
		o.AddMember(StringRef(temps.c_str()), sub_o, d.GetAllocator());

		(*_output_str) << Json::stringify(o);
	} else
	if(getRepr() == Binary) {
		for(auto &m : messages) {
			// cout << "==============================\n";
			// cout << m->GetTypeName() << "\n";
			// cout << m->DebugString();
			writeBinaryMessage(m, _output_str);
		}
	}

}

#define P(condition) {if(!condition) { printf( "\n FAILURE in %s, line %d\n", __FILE__, __LINE__ );exit( 1 );}}

void Stream::protoReader(vector<ProtoMessage> &messages) {
	Protos::ClassName *cl = new Protos::ClassName;
	P(readBinaryMessage(cl, _input_str));
	
	messages.push_back(cl);
	
	if(cl->has_proto()) {
		ProtoMessage pr = Factory::inst().createProto(cl->class_name());
		P(readBinaryMessage(pr, _input_str));		
		messages.push_back(pr);
	}

	for(size_t i=0; i<cl->size(); ++i) {
		protoReader(messages);
	}

}

void Stream::jsonReader(string name, const Value &v, vector<ProtoMessage> &messages) {
	// json to protobuf
	Protos::ClassName *cl = new Protos::ClassName;
	cl->set_class_name(name);
	messages.push_back(cl);

	bool has_object = false;
	for (Value::ConstMemberIterator itr = v.MemberBegin(); itr != v.MemberEnd(); ++itr) {
		if(itr->value.IsObject()) {
			jsonReader(itr->name.GetString(), itr->value, messages);
			has_object = true;
		}
	}
	if(!has_object) {
		if(!Factory::inst().isProtoType(name)) {
			// trying to deduce proto name
			if(!Factory::inst().isProtoType(name + string("C"))) {
				cerr <<  "Erros while reading " << name << ": unkwnown proto type\n";
				terminate();
			}
			name += string("C");
			Protos::ClassName *ccl = new Protos::ClassName;
			ccl->set_class_name(name);
			messages.push_back(ccl);
		}
		ProtoMessage m = Factory::inst().createProto(name);
		string err;
		pbjson::jsonobject2pb(&v, m, err);
		if(!err.empty()) {
			cerr << "Found errors while converting json to protobuf:\n";
			cerr << err << "\n";
			terminate();
		}

		messages.push_back(m);
	}
}

SerializableBase* Stream::readObject() {

	if(!isInput()) {
		cerr << "Stream isn't open in input mode. Need input stream\n";
		terminate();
	}

	vector<ProtoMessage> messages;
	if(r == Binary) {
		protoReader(messages);
	}
	if(r == Text) {
		if(!iterator->value.IsObject()) {
			cerr << "Fail to read " << Json::stringify(iterator->value) << ". Expected object\n";
			terminate();	
		}
		if(iterator == document.MemberEnd()) {
			return nullptr;
		}
		jsonReader(iterator->name.GetString(), iterator->value, messages);
		iterator++;
	}

	assert(messages.size()>0);
	for(auto &m : messages) {
		cout << m->GetTypeName() << " =================\n";
		cout << m->DebugString();
	}
	std::reverse(messages.begin(), messages.end());
	
	Protos::ClassName *head = SerializableBase::getHeader(messages);

	SerializableBase *o = Factory::inst().createObject(head->class_name());
	o->getDeserialized(messages);
	return o;
}


}


