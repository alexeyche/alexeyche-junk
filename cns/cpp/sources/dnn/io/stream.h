#pragma once

#include <dnn/core.h>
#include <dnn/io/serialize.h>

#include <google/protobuf/io/zero_copy_stream_impl.h>
#include <google/protobuf/io/coded_stream.h>


namespace dnn {

using namespace google::protobuf::io;

class SerializableBase;

class Stream {
public:
    enum Repr { Binary, Text };
    
    Stream(istream &str, Repr _r = Binary, bool _destroy_stream=false) 
    : _input_str(&str)
    , r(_r)
    , destroy_stream(_destroy_stream)
    , _output_str(nullptr)
    , zeroOut(nullptr)
    , codedOut(nullptr)
    ,zeroIn(nullptr)
    , codedIn(nullptr) 
    {
        if((_input_str)&&(!_input_str->good())) {
            throw dnnException()<< "Input filestream isn't open\n";
        }
        if(r == Binary) {        
            zeroIn = new IstreamInputStream(_input_str);
            codedIn = new CodedInputStream(zeroIn);
            codedIn->SetTotalBytesLimit(300.0 * 1024 * 1024,300.0 * 1024 * 1024);
        }
        if(r == Text) {
            string jstr((std::istreambuf_iterator<char>(*_input_str)), std::istreambuf_iterator<char>());
            document = Json::parseString(jstr);
            assert(document.IsObject());
            iterator = document.MemberBegin();
        }
    }
    
    Stream(ostream &str, Repr _r = Binary, bool _destroy_stream=false) 
    : _output_str(&str)
    , r(_r)
    , _input_str(nullptr)
    , destroy_stream(_destroy_stream)
    , zeroIn(nullptr)
    , codedIn(nullptr)
    , zeroOut(nullptr)
    , codedOut(nullptr) 
    {
        if((_output_str)&&(!_output_str->good())) {
            throw dnnException()<< "Output filestream isn't open\n";
        }
        if(r == Binary) {
            zeroOut = new OstreamOutputStream(_output_str);
            codedOut = new CodedOutputStream(zeroOut);
        }
    }
    ~Stream() {
        if(destroy_stream) {
            if(_output_str) delete _output_str;
            if(_input_str) delete _input_str;
        }
        if(codedIn)  delete codedIn;
        if(zeroIn)   delete zeroIn;
        if(codedOut) delete codedOut;
        if(zeroOut)  delete zeroOut;
    }
    bool isOutput() {
        if(_output_str) return true;
        return false;
    }
    bool isInput() {
        if(_input_str) return true;
        return false;
    }
    istream& getInputStream() {
        if(_input_str) return *_input_str;
        throw dnnException()<< "Stream is wrongly opened or used\n";
    }
    ostream& getOutputStream() {
        if(_output_str) return *_output_str;
        throw dnnException()<< "Stream is wrongly opened or used\n";
    }

    void writeObject(SerializableBase *b);
    SerializableBase* readBaseObject(SerializableBase *o = nullptr);    
        
    template <typename T>
    T* readObject(T *o = nullptr) {
        SerializableBase *b = readBaseObject(o);
        if(!b) return nullptr;
        
        T *d = dynamic_cast<T*>(b);
        if(!d) {
            throw dnnException()<< "Failed to cast from " << b->name() << "\n";
        }
        return d;
    }
    vector<ProtoMessage> readObjectProtos();
    void protoReader(vector<ProtoMessage> &messages);
    void jsonReader(string name, const Value &v, vector<ProtoMessage> &messages);
    
    Repr getRepr() {
        return r;
    }

    bool readBinaryMessage(ProtoMessage mess, istream *str) {        
        google::protobuf::uint32 size;
        if(!codedIn->ReadVarint32(&size)) {
                return false;
        }
        CodedInputStream::Limit limit = codedIn->PushLimit(size);
        if(!mess->ParseFromCodedStream(codedIn)) {
                throw dnnException()<< "Can't parse message with size " << size << "\n";
        }
        codedIn->PopLimit(limit);
        return true;
    }

    void writeBinaryMessage(ProtoMessage mess, ostream *str) {
        if(!mess) {
            throw dnnException()<< "Trying to write null binary message\n";
        }
        google::protobuf::uint32 size = mess->ByteSize();
        codedOut->WriteVarint32(size);
        mess->SerializeToCodedStream(codedOut);
    }

private:
    istream *_input_str;
    ostream *_output_str;

    OstreamOutputStream *zeroOut;
    CodedOutputStream *codedOut;

    IstreamInputStream *zeroIn;
    CodedInputStream *codedIn;

    Repr r;
    Document document;
    Value::ConstMemberIterator iterator;
    bool destroy_stream;
};

}
