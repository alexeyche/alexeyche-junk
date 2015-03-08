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
		// Document d;
		// Value o(kObjectType);
		// Value sub_o(kObjectType);
		// for(auto &m: messages) {
		// 	Document *sub_d = Json::parseProtobuf(m.second);
		// 	sub_o.AddMember(StringRef(m.first.c_str()), *sub_d, d.GetAllocator());
		// }
		// o.AddMember(StringRef(cl.class_name().c_str()), sub_o, d.GetAllocator());

		// (*_output_str) << Json::stringify(o);
	} else
	if(getRepr() == Binary) {
		for(auto &m : messages) {
			cout << "==============================\n";
			cout << m->GetTypeName() << "\n";
			cout << m->DebugString();
			writeBinaryMessage(m, _output_str);
		}
	}

}

#define P(condition) {if(!condition) { printf( "\n FAILURE in %s, line %d\n", __FILE__, __LINE__ );exit( 1 );}}

void Stream::reader(vector<ProtoMessage> &messages) {
	Protos::ClassName *cl = new Protos::ClassName;
	P(readBinaryMessage(cl, _input_str));
	
	cout << "Read " << cl->GetTypeName() << "\n";
	cout << cl->DebugString();
	cout << "==================\n";
	
	messages.push_back(cl);
	
	if(cl->has_proto()) {
		Protos::ClassName *sub_cl = new Protos::ClassName;
		P(readBinaryMessage(sub_cl, _input_str));
		
		cout << "Read " << sub_cl->GetTypeName() << "\n";
		cout << sub_cl->DebugString();
		cout << "==================\n";

		SerializableBase *o = Factory::inst().createObject(sub_cl->class_name());
		
		ProtoMessage pr = o->newProto();
		
		P(readBinaryMessage(pr, _input_str));		
	
		cout << "Read " << pr->GetTypeName() << "\n";
		cout << pr->DebugString();
		cout << "==================\n";

		messages.push_back(pr);

		Factory::inst().deleteLast();
	}

	for(size_t i=0; i<cl->size(); ++i) {
		reader(messages);
	}

}



SerializableBase* Stream::readObject() {
	if(!isInput()) {
		cerr << "Stream isn't open in input mode. Need input stream\n";
		terminate();
	}
	cout << "=============================\n===============================\n";
	vector<ProtoMessage> messages;
	reader(messages);


	return nullptr;
}


}


