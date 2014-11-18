#pragma once

#include <snnlib/protos/time_series.pb.h>
#include <snnlib/protos/common.pb.h>
#include <google/protobuf/io/zero_copy_stream_impl.h>
#include <google/protobuf/io/coded_stream.h>

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
            fd = open(filename.c_str(), O_RDONLY);
            if (fd == -1) {
                cerr << "Error while opening file " << filename << "\n";
                terminate();
            }

            ifs_g = new google::protobuf::io::FileInputStream(fd);
            codedIn = new CodedInputStream(ifs_g);
        } else
        if(m == Write) {
            ofs = new ofstream(filename, ios::out | ios::trunc | ios::binary);
        }
    }
    ~ProtoRw() {
        if(m == Read) {
            close(fd);
            delete codedIn;
            delete ifs_g;

        } else
        if(m == Write) {
            ofs->close();
            delete ofs;
        }

    }
    Serializable* readAny() {
        CHECK_MODE(Read);
        Protos::ClassName cl;
        if(!readMessage(cl)) {
            cerr << "Can't read ClassName from " << filename << "\n";
            terminate();
        }
        Serializable *s = SerializableFactory::inst().create(cl.name());
        google::protobuf::Message *mess = s->getNew();
        readMessage(*mess);
        s->deserialize();
        return s;
    }
    void readAndPrintAny() {
        CHECK_MODE(Read);
        Protos::ClassName cl;
        if(!readMessage(cl)) {
            cerr << "Can't read ClassName from " << filename << "\n";
            terminate();
        }
        Serializable *s = SerializableFactory::inst().create(cl.name());
        google::protobuf::Message *mess = s->getNew();
        readMessage(*mess);
        cout << mess->DebugString();
    }
    bool read(Serializable *s) {
        CHECK_MODE(Read);
        Protos::ClassName cl;
        if(!readMessage(cl)) {
            return false;
        }
        if(cl.name() != s->getName()) {
            cerr << "Trying to read " << cl.name() << " as " << s->getName() << "\n";
            terminate();
        }
        google::protobuf::Message *mess = s->getNew();
        readMessage(*mess);
        s->deserialize();
        return true;
    }

    void write(Serializable *s) {
        CHECK_MODE(Write);
        Protos::ClassName cl;
        cl.set_name(s->getName());
        writeMessage(&cl);
        ::google::protobuf::Message *mess = s->serialize();
        writeMessage(mess);
    }


    template <typename T>
    vector<T> readAll() {
        CHECK_MODE(Read);
        vector<T> v;
        while(true) {
            T el;
            if(!read(el)) {
                break; // we are at end
            }
            v.push_back(el);
        }
        return v;
    }

private:
    void writeMessage(::google::protobuf::Message *message) {
        zeroOut = new OstreamOutputStream(ofs);
        codedOut = new CodedOutputStream(zeroOut);

        google::protobuf::uint32 size = message->ByteSize();

        codedOut->WriteVarint32(size);
        message->SerializeToCodedStream(codedOut);

        delete codedOut;
        delete zeroOut;
    }
    bool readMessage(::google::protobuf::Message &message) {

        google::protobuf::uint32 size;
        if(!codedIn->ReadVarint32(&size)) {
            return false;
        }
        CodedInputStream::Limit limit = codedIn->PushLimit(size);
        if(!message.ParseFromCodedStream(codedIn)) {
            cerr << "Can't parse message with size " << size << "\n";
            terminate();
        }
        codedIn->PopLimit(limit);

        return true;
    }

    int fd;


    Mode m;
    ofstream *ofs;
    OstreamOutputStream *zeroOut;
    CodedOutputStream *codedOut;

    ifstream *ifs;
    google::protobuf::io::FileInputStream* ifs_g;

    IstreamInputStream *zeroIn;
    CodedInputStream *codedIn;

    string filename;

};
