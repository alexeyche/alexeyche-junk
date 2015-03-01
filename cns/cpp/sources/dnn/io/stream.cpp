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
	Protos::ClassName cl;
	cl.set_class_name(b->name());
	vector<ProtoMessage> messages = b->getSerialized();
	
}

SerializableBase* readObject() {
	return nullptr;
}


}


