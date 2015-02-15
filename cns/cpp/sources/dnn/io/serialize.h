#pragma once

#include <dnn/io/stream.h>
#include <dnn/core.h>
#include <dnn/util/util.h>

#include <google/protobuf/message.h>
#include <google/protobuf/io/zero_copy_stream_impl.h>
#include <google/protobuf/io/coded_stream.h>

#include <dnn/contrib/pbjson/pbjson.hpp>
#include <dnn/contrib/rapidjson/stringbuffer.h>
#include <dnn/contrib/rapidjson/prettywriter.h>

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
	
	Serializable() : _ack_stream(nullptr), mess(nullptr) {}
	~Serializable() {
		if(mess) delete mess;
	}
    virtual void processStream(Stream &str) = 0;
  	
  	Serializable& acquire(Stream &str) {
  		_ack_stream = &str;

  		return *this;
  	}
  	static void readBinaryMessage(google::protobuf::Message* mess, istream *str) {
  		if(mess) {
  			delete mess;
  		}
  		mess = new Proto;

  		IstreamInputStream *zeroIn = new IstreamInputStream(str);
  		CodedInputStream *codedIn = new CodedInputStream(zeroIn);
  		
  	    google::protobuf::uint32 size;
        if(!codedIn->ReadVarint32(&size)) {
            return false;
        }
        CodedInputStream::Limit limit = codedIn->PushLimit(size);
        if(!mess->ParseFromCodedStream(codedIn)) {
            cerr << "Can't parse message with size " << size << "\n";
            terminate();
        }
        codedIn->PopLimit(limit);
  	}

  	static void writeBinaryMessage(google::protobuf::Message* mess, ostream *str) {
		if(!mess) {
			cerr << "Trying to write null binary message\n";
		}
  		google::protobuf::uint32 size = mess->ByteSize();
  		OstreamOutputStream *zeroOut = new OstreamOutputStream(str);
  		CodedOutputStream *codedOut = new CodedOutputStream(zeroOut);
  		
  		codedOut->WriteVarint32(size);  		
  		mess->SerializeToCodedStream(codedOut);

  		delete codedOut;
  		delete zeroOut;
  	}

  	Serializable& operator << (EndMarker v) {
  		ASSERT_FIELDS()
  		if(_ack_stream->isOutput()) {
	  		if(_ack_stream->getRepr() == Stream::Binary) {
	  			writeBinaryMessage(mess, &_ack_stream->getOutputStream());
	  		} else
	  		if(_ack_stream->getRepr() == Stream::Text) {
	  		    rapidjson::Value* v = pbjson::pb2jsonobject(mess);
	  		    std::string str;
	            rapidjson::StringBuffer buffer;
		        rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(buffer);
    		    v->Accept(writer);
        		str.append(buffer.GetString(), buffer.GetSize());
	  			_ack_stream->getOutputStream() << str;
	  		} else {
	  			cerr << "UB\n";
	  			terminate();
	  		}
	  	} else 
  		if(_ack_stream->isInput()) {
  			if(_ack_stream->getRepr() == Stream::Binary) {
  			} else
	  		if(_ack_stream->getRepr() == Stream::Text) {

	  		} else {
	  			cerr << "UB\n";
	  			terminate();
	  		}
  		} else {
  			cerr << "UB\n";
  			terminate();
  		}
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
  		if(_ack_stream->isOutput()) {
  			reflection->SetDouble(mess, field_descr, v);  		
  		} else {
  			v = reflection->GetDouble(*mess, field_descr);  		
  		}
  		return *this;
  	}
  	Serializable& operator << (size_t &v) {
  		ASSERT_FIELDS()
  		if(_ack_stream->isOutput()) {
  			reflection->SetUInt32(mess, field_descr, v);
  		} else {
  			v = reflection->GetUInt32(*mess, field_descr);  			
  		}
  		return *this;
  	}
  	Serializable& operator << (string &v) {
  		ASSERT_FIELDS()
  		if(_ack_stream->isOutput()) {
			reflection->SetString(mess, field_descr, v);
		} else {
			v = reflection->GetString(*mess, field_descr);
		}
		return *this;
  	}

  	
private:
	const google::protobuf::FieldDescriptor* field_descr;
	const google::protobuf::Reflection* reflection;

	string current_field;
	google::protobuf::Message* mess;
	Stream *_ack_stream;  	
    
};


}