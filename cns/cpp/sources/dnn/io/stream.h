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

    Stream(istream &str, Repr _r = Binary) : _input_str(&str), r(_r), _output_str(nullptr), zeroOut(nullptr), codedOut(nullptr) {
        if((_input_str)&&(!_input_str->good())) {
            cerr << "Input filestream isn't open\n";
            terminate();
        }
        if(r == Binary) {        
            zeroIn = new IstreamInputStream(_input_str);
            codedIn = new CodedInputStream(zeroIn);
            codedIn->SetTotalBytesLimit(300.0 * 1024 * 1024,300.0 * 1024 * 1024);
        }



    }
    Stream(ostream &str, Repr _r = Binary) : _output_str(&str), r(_r), _input_str(nullptr), zeroIn(nullptr), codedIn(nullptr) {
        if((_output_str)&&(!_output_str->good())) {
            cerr << "Output filestream isn't open\n";
            terminate();
        }
        if(r == Binary) {
            zeroOut = new OstreamOutputStream(_output_str);
            codedOut = new CodedOutputStream(zeroOut);
        }
    }
    ~Stream() {
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
        cerr << "Stream is wrongly opened or used\n";
        terminate();
    }
    ostream& getOutputStream() {
        if(_output_str) return *_output_str;
        cerr << "Stream is wrongly opened or used\n";
        terminate();
    }

    void writeObject(SerializableBase *b);
    SerializableBase* readObject();
    void protoReader(vector<ProtoMessage> &messages);
    void jsonReader(Value &v, vector<ProtoMessage> &messages);
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
                cerr << "Can't parse message with size " << size << "\n";
                terminate();
        }
        codedIn->PopLimit(limit);
        return true;
    }

    void writeBinaryMessage(ProtoMessage mess, ostream *str) {
        if(!mess) {
            cerr << "Trying to write null binary message\n";
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
};

}
