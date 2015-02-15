#pragma once

#include <dnn/io/stream.h>
#include <dnn/core.h>
#include <dnn/util/util.h>

#include <google/protobuf/message.h>
#include <google/protobuf/io/zero_copy_stream_impl.h>
#include <google/protobuf/io/coded_stream.h>

using namespace google::protobuf::io;
namespace dnn {


template <typename Proto>
class Serializable {
public: 
	#define ASSERT_FIELDS() \
	if((!reflection)||(!mess)||(!field_descr)) {\
		cerr << "Wrong using of Serializable class.\n"; \
		terminate(); \
	}\

	typedef Serializable<Proto> Self;
	
	enum EndMarker { End };	
	
	Serializable() : _ack_ostream(nullptr), mess(nullptr) {}
	~Serializable() {
		if(mess) delete mess;
	}
    virtual void out(ostream &str) = 0;
  	
  	Serializable& serialize(ostream &str) {
  		_ack_ostream = &str;

  		return *this;
  	}
  	Serializable& operator << (EndMarker v) {
  		google::protobuf::uint32 size = mess->ByteSize();
  		OstreamOutputStream *zeroOut = new OstreamOutputStream(_ack_ostream);
  		CodedOutputStream *codedOut = new CodedOutputStream(zeroOut);
  		
  		codedOut->WriteVarint32(size);  		
  		mess->SerializeToCodedStream(codedOut);

  		delete codedOut;
  		delete zeroOut;

  		return *this;
  	}
  	
  	Serializable& operator << (string v) {
  		if(!mess) {
  			mess = new Proto;
  			reflection = mess->GetReflection();
  		}
  		vector<string> v_spl = split(v, ':');
  		string fname = v_spl[0];
  		trim(fname);

  		const google::protobuf::Descriptor* descriptor = mess->GetDescriptor();
  		field_descr = descriptor->FindFieldByName(fname);
  		if(!field_descr) {
  			cerr << "Can't find proto field by name " << fname << "\n";	
  			terminate();  			
  		} 
  		return *this;
  	}

  	Serializable& operator << (double &v) {
  		ASSERT_FIELDS()
  		reflection->SetDouble(mess, field_descr, v);  		
  		return *this;
  	}
  	Serializable& operator << (size_t &v) {
  		ASSERT_FIELDS()
  		reflection->SetUInt32(mess, field_descr, v);
  		return *this;
  	}
  	Serializable& operator << (string &v) {
  		ASSERT_FIELDS()
		reflection->SetString(mess, field_descr, v);
		return *this;
  	}

  	
private:
	const google::protobuf::FieldDescriptor* field_descr;
	const google::protobuf::Reflection* reflection;

	string current_field;
	google::protobuf::Message* mess;
	ostream *_ack_ostream;  	
    
};


}