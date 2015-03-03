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

	vector<NamedMessage> messages = b->getSerialized();
	Protos::ClassName cl;
	cl.set_class_name(b->name());
	cl.set_size(messages.size());

	if(getRepr() == Text) {
		Document d;
		Value o(kObjectType);
		Value sub_o(kObjectType);
		for(auto &m: messages) {
			Document *sub_d = Json::parseProtobuf(m.second);
			sub_o.AddMember(StringRef(m.first.c_str()), *sub_d, d.GetAllocator());
		}
		o.AddMember(StringRef(cl.class_name().c_str()), sub_o, d.GetAllocator());

		(*_output_str) << Json::stringify(o);
	} else
	if(getRepr() == Binary) {
		writeBinaryMessage(&cl, _output_str);
		for(auto &m : messages) {
			Protos::ClassName sub_cl;
			sub_cl.set_class_name(m.first);
			writeBinaryMessage(&sub_cl, _output_str);
			writeBinaryMessage(m.second, _output_str);
		}
	}

}

#define P(condition) {if(!condition) { printf( "\n FAILURE in %s, line %d\n", __FILE__, __LINE__ );exit( 1 );}}

SerializableBase* Stream::readObject() {
	if(!isInput()) {
		cerr << "Stream isn't open in input mode. Need input stream\n";
		terminate();
	}
	Protos::ClassName cl;
	P(readBinaryMessage(&cl, _input_str));

	assert(cl.has_size());
	SerializableBase* o = Factory::inst().createObject(cl.class_name());

	vector<NamedMessage> messages;
	for(size_t i=0; i<cl.size(); ++i) {
		Protos::ClassName sub_cl;
		P(readBinaryMessage(&sub_cl, _input_str));
		cout << "Creating " << sub_cl.class_name() << "\n";
		SerializableBase *sub_o = Factory::inst().createObject(sub_cl.class_name());
		ProtoMessage m = sub_o->newProto();
		P(readBinaryMessage(m, _input_str));
		messages.push_back( NamedMessage(sub_o->name(), m) );
	}
	std::reverse(messages.begin(), messages.end());
	o->getDeserialized(messages);

	return o;
}


}


