#pragma once

#pragma once

#include <snnlib/protos/time_series.pb.h>
#include <google/protobuf/io/zero_copy_stream_impl.h>
#include <google/protobuf/io/coded_stream.h>

#include <fcntl.h>

using namespace google::protobuf::io;



class ProtoRw {
public:    
    #define CHECK_MODE(mode) \
        if(m != mode) {\
            cerr << "Can't do it in that mode\n";\
            terminate();\
        }\

    enum Mode {Read, Write};
    ProtoRw(const string &f, Mode _m) : m(_m) {
        if(m == Read) {
            ifs = new ifstream(f, ios::in | ios::binary);
            zeroIn = new IstreamInputStream(ifs);
            codedIn = new CodedInputStream(zeroIn);
        } else
        if(m == Write) {
            ofs = new ofstream(f, ios::out | ios::trunc | ios::binary);
            zeroOut = new OstreamOutputStream(ofs);
            codedOut = new CodedOutputStream(zeroOut);
        }
    }
    ~ProtoRw() {
        if(m == Read) {
            delete codedIn;
            delete zeroIn;
            delete ifs;
            ifs->close();
        } else
        if(m == Write) {
            delete codedOut;
            delete zeroOut;
            ofs->close();
            delete ofs;
        }

    }
    template <typename T>
    T read() {
        CHECK_MODE(Read);
        T lts;
        readMessage(lts);
        return lts;
    }
    template <typename T>
    void write(T &lts) {
        CHECK_MODE(Write);
        writeMessage(lts);
    }

    template <typename T>
    vector<T> readAll() {
        CHECK_MODE(Read);
        vector<T> v;
        while(!ifs->eof()) {
            T el = read<T>();
            v.push_back(el);
        }
        return v;
    }

private:    
    void writeMessage(::google::protobuf::Message &message) {
        string data;
        message.SerializeToString(&data);
        codedOut->WriteVarint32((int) data.length());
        codedOut->WriteRaw(data.c_str(), data.length());
    }
    void readMessage(::google::protobuf::Message &message) {
        google::protobuf::uint32 size;
        if (!codedIn->ReadVarint32(&size)) { 
            cerr << "Failed to read size of message\n";
            terminate();
        }
        std::string buffer;
        codedIn->ReadString(&buffer, size);
        message.ParseFromString(buffer);
    }




    Mode m;
    ofstream *ofs;
    OstreamOutputStream *zeroOut;
    CodedOutputStream *codedOut;

    ifstream *ifs;
    IstreamInputStream *zeroIn;
    CodedInputStream *codedIn;

};
