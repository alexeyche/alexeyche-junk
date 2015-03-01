#pragma once

#include <dnn/io/stream.h>
#include <dnn/core.h>
#include <dnn/util/util.h>
#include <dnn/util/json.h>
#include <dnn/base/base.h>
#include <dnn/base/factory.h>

#include <google/protobuf/message.h>
#include <google/protobuf/io/zero_copy_stream_impl.h>
#include <google/protobuf/io/coded_stream.h>


#include <dnn/protos/generated.pb.h>

using namespace google::protobuf::io;

typedef google::protobuf::Message* ProtoMessage;

namespace dnn {


class SerializableBase  : public Object {
friend class Factory;    
public:
    enum EndMarker { End };

    SerializableBase() : output(true) {
    
    }
    virtual const string name() const = 0;   
    virtual ~SerializableBase() {
        for(auto &m: messages) {
            delete m;
        }
    }
    
    virtual void serialize() = 0;
    

    static bool readBinaryMessage(google::protobuf::Message* mess, istream *str) {
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
        return true;
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

    void operator << (EndMarker e) {
    }
    
    SerializableBase& begin() {
        return *this;
    }
    EndMarker end() {
        return End;
    }

    static ProtoMessage copyM(ProtoMessage m) {
        ProtoMessage copy_m = m->New();
        copy_m->CopyFrom(*m);
        return copy_m;
    }
    
    SerializableBase& operator << (SerializableBase &b) {
        for(auto &m: b.getSerialized()) {
            messages.push_back(copyM(m));
        }
        return *this;
    }

    vector<ProtoMessage>& getSerialized() {
        output = true;
        serialize();
        return messages;
    }
    
    SerializableBase& operator << (const char *vraw) {
        return *this;
    }

protected:
    vector<ProtoMessage> messages;
    bool output;      
};


template <typename Proto>
class Serializable : public SerializableBase {
public:
    #define ASSERT_FIELDS() \
    if((!reflection)||(!messages.size() != 0)||(!field_descr)) {\
        cerr << "Wrong using of Serializable class.\n"; \
        terminate(); \
    }\

    typedef Serializable<Proto> Self;

    const string name() const {
        Proto _fake_m;
        vector<string> spl = split(_fake_m.GetTypeName(), '.');
        if(spl[0] != "Protos") {
            cerr << "Expection Protos:: typename\n";
            terminate();
        }
        string ret;
        for(size_t i=1; i<spl.size(); ++i) {
            ret += spl[i];
        }
        return ret;
    }

    // Serializable& operator << (EndMarker v) {
    //     ASSERT_FIELDS()
    //     if(_ack_stream->isOutput()) {
    //         if(_ack_stream->getRepr() == Stream::Binary) {
    //             writeBinaryMessage(mess, &_ack_stream->getOutputStream());
    //         } else
    //         if(_ack_stream->getRepr() == Stream::Text) {
    //             Document *v = Json::parseProtobuf(mess);
                
    //             // rapidjson::Value o(kObjectType);
    //             // string class_name = name();
    //             // Value class_name_val(class_name.c_str(), class_name.size(), v->GetAllocator());
            
    //             //o.AddMember(class_name_val, *v, v->GetAllocator());
    //             _ack_stream->getOutputStream() << Json::stringify(*v);
    //             delete v;
    //         } else {
    //             cerr << "UB\n";
    //             terminate();
    //         }
    //     } else
    //     if(_ack_stream->isInput()) {
    //         mess = new Proto;
    //         if(_ack_stream->getRepr() == Stream::Binary) {
    //             readBinaryMessage(mess, &_ack_stream->getInputStream());
    //         } else
    //         if(_ack_stream->getRepr() == Stream::Text) {
    //             std::istreambuf_iterator<char> eos;
    //             string s(std::istreambuf_iterator<char>(_ack_stream->getInputStream()), eos);
    //             string err;
    //             pbjson::json2pb(s, mess, err);
    //             if(!err.empty()) {
    //                 cerr << "Errors while converting json:\n";
    //                 cerr << err;
    //                 terminate();
    //             }
    //         } else {
    //             cerr << "UB\n";
    //             terminate();
    //         }
    //     } else {
    //         cerr << "UB\n";
    //         terminate();
    //     }
    //     return *this;
    // }

    Serializable& operator << (const char *vraw) {
        if(messages.size() == 0) {
            cerr << "Serialaling without begin()\n";
            terminate();
        }

        string v = string(vraw);
        if(trimC(v) == ",") return *this;

        vector<string> v_spl = split(v, ':');
        string fname = v_spl[0];
        trim(fname);
        
        const google::protobuf::Descriptor* descriptor = messages.back()->GetDescriptor();
        field_descr = descriptor->FindFieldByName(fname);
        if(!field_descr) {
            cerr << "Can't find proto field by name " << fname << "\n";
            terminate();
        }
        return *this;
    }
    Serializable& operator << (double &v) {
        ASSERT_FIELDS()
        if(output) {
            reflection->SetDouble(messages.back(), field_descr, v);
        } else {
            v = reflection->GetDouble(*messages.back(), field_descr);
        }
        return *this;
    }

    Serializable& operator << (bool &v) {
        ASSERT_FIELDS()
        if(output) {
            reflection->SetBool(messages.back(), field_descr, v);
        } else {
            v = reflection->GetBool(*messages.back(), field_descr);
        }
        return *this;
    }

    Serializable& operator << (size_t &v) {
        ASSERT_FIELDS()
        if(output) {
            reflection->SetUInt32(messages.back(), field_descr, v);
        } else {
            v = reflection->GetUInt32(*messages.back(), field_descr);
        }
        return *this;
    }

    Serializable& operator << (string &v) {
        ASSERT_FIELDS()
        if(output) {
           reflection->SetString(messages.back(), field_descr, v);
        } else {
            v = reflection->GetString(*messages.back(), field_descr);
        }
        return *this;
    }

    void operator << (EndMarker e) {

    }
    Serializable& begin() {
        ProtoMessage mess = new Proto;
        reflection = mess->GetReflection();
        messages.push_back(mess);
        return *this;
    }
    EndMarker end() {
        return End;
    }
private:
    const google::protobuf::FieldDescriptor* field_descr;
    const google::protobuf::Reflection* reflection;
};



}
