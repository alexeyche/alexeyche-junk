#pragma once

#include <snnlib/protos/time_series.pb.h>
#include <snnlib/protos/common.pb.h>
#include <google/protobuf/io/zero_copy_stream_impl.h>
#include <google/protobuf/io/coded_stream.h>

#include <snnlib/config/factory.h>

#include <fcntl.h>
#include <unistd.h>
using namespace google::protobuf::io;

static int glob_deb = 0;


class ProtoRw {
public:
    #define CHECK_MODE(mode) \
        if(m != mode) {\
            cerr << "Can't do it in that mode\n";\
            terminate();\
        }\

    enum Mode {Read, Write};
    ProtoRw(const string &f, Mode _m) : filename(f), m(_m) {
        if(m == Read) {
            ifs = new ifstream(f, ios::in | ios::binary);
            if(!ifs->is_open()) {
                cerr << "Failed to open file " << f << " for read\n";
                terminate();
            }
            zeroIn = new IstreamInputStream(ifs);
            codedIn = new CodedInputStream(zeroIn);
            codedIn->SetTotalBytesLimit(100.0 * 1024 * 1024,100.0 * 1024 * 1024);
        } else
        if(m == Write) {
            if(f.empty()) {
                cerr << "Can'write to empty file\n";
                terminate();
            }
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
        } else
        if(m == Write) {
            delete codedOut;
            delete zeroOut;
            delete ofs;
        }

    }

    void readAllocated(SerializableBase *b) {
        CHECK_MODE(Read);
        Protos::Class cl;
        if(!readMessage(&cl)) {
            cerr << "Error while reading allocated instance: can't read Class from stream\n";
            terminate();
        }
        if(cl.instance_class_name() != b->getName()) {
            cerr << "Error while reading allocated instance: serialized and allocated names are not equal\n";
            cerr << "\t" << cl.instance_class_name() << " != " << b->getName() << "\n";
            terminate();
        }

        ProtoPack pack = b->getNew();
        for(size_t mi=0; mi<pack.size(); mi++) {
            readMessage(pack[mi]);
        }
        b->deserialize();
    }

    SerializableBase* read(bool print=false) {
        CHECK_MODE(Read);
        Protos::Class cl;
        if(!readMessage(&cl)) {
            return nullptr;
        }
        if(print) {
            cout << cl.DebugString();
        }
        SerializableBase *main_inst = Factory::inst().createSerializable(cl.instance_class_name());
        ProtoPack pack = main_inst->getNew();
        for(size_t mi=0; mi<pack.size(); mi++) {
            readMessage(pack[mi]);
            if(print) {
                cout << pack[mi]->DebugString();
            }
        }
        main_inst->deserialize();
        return main_inst;
    }
    
    void write(SerialPack pack) {
        for(auto it=pack.begin(); it != pack.end(); ++it) {
            write(*it);
        }
    }
        
    void write(SerializableBase *mess) {
        CHECK_MODE(Write);
        Protos::Class cl;
        cl.set_instance_class_name(mess->getName());
        writeMessage(&cl);
        ProtoPack pack = mess->serialize();
        for(size_t mi=0; mi<pack.size(); mi++) {
            writeMessage(pack[mi]);
        }
    }

    template <typename T>
    vector<T> readAll() {
        CHECK_MODE(Read);
        vector<T> v;
        while(true) {
            T el;
            if(!read(&el)) {
                break; // we are at end
            }
            v.push_back(el);
        }
        return v;
    }

private:
    void writeMessage(::google::protobuf::Message *message) {
        google::protobuf::uint32 size = message->ByteSize();

        codedOut->WriteVarint32(size);
        message->SerializeToCodedStream(codedOut);
    }
    bool readMessage(::google::protobuf::Message *message) {
        google::protobuf::uint32 size;
        if(!codedIn->ReadVarint32(&size)) {
            return false;
        }
        CodedInputStream::Limit limit = codedIn->PushLimit(size);
        if(!message->ParseFromCodedStream(codedIn)) {
            cerr << "Can't parse message with size " << size << "\n";
            terminate();
        }
        codedIn->PopLimit(limit);
        return true;
    }
private:

    Mode m;

    ofstream *ofs;
    OstreamOutputStream *zeroOut;
    CodedOutputStream *codedOut;

    ifstream *ifs;
    IstreamInputStream *zeroIn;
    CodedInputStream *codedIn;

    string filename;

};
