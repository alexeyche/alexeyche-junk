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
    SerialFamily readAny(bool print=false) {
        CHECK_MODE(Read);
        Protos::Family pack;
        if(!readMessage(pack)) {
            return SerialFamily();
        }
        SerialFamily ps;
        for(size_t mi=0; mi<pack.names_size(); mi++) {
            Protos::ClassName cl = pack.names(mi);
            Serializable *s = SerializableFactory::inst().create(cl.name());
            google::protobuf::Message *mess = s->getNew();
            readMessage(*mess);
            if(print) {
                cout << mess->DebugString();
            }
            s->deserialize();
            ps.push_back(s);
        }
        return ps;
    }


    bool read(SerialFamily ps) {
        CHECK_MODE(Read);
        Protos::Family pack;
        if(!readMessage(pack)) {
            return false;
        }

        if(ps.size() != pack.names_size()) {
            cerr << "Trying to read pack of messages with different size " << ps.size() << " != " << pack.names_size() << "\n";
            terminate();
        }
        for(size_t mi=0; mi<pack.names_size(); mi++) {
            Serializable *s = ps[mi];
            Protos::ClassName cl = pack.names(mi);
            if(cl.name() != s->getName()) {
                cerr << "Trying to read " << cl.name() << " as " << s->getName() << "\n";
                terminate();
            }
            google::protobuf::Message *mess = s->getNew();
            readMessage(*mess);
            s->deserialize();
        }
        return true;
    }
    bool read(Serializable* s) {
        return read(SerialFamily({s}));
    }
    void write(SerialPack &ps) {
        for(auto it=ps.begin(); it != ps.end(); ++it) {
            write(*it);
        }
    }
    void write(SerialFamily ps) {
        CHECK_MODE(Write);
        Protos::Family pack;
        for(size_t mi=0; mi<ps.size(); mi++) {
            Serializable *s = ps[mi];
            Protos::ClassName *cl = pack.add_names();
            cl->set_name(s->getName());
        }
        writeMessage(&pack);
        for(size_t mi=0; mi<ps.size(); mi++) {
            Serializable *s = ps[mi];
            ::google::protobuf::Message *mess = s->serialize();
            writeMessage(mess);
        }
    }
    void write(Serializable* s) {
        write(SerialFamily({s}));
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
        google::protobuf::uint32 size = message->ByteSize();

        codedOut->WriteVarint32(size);
        message->SerializeToCodedStream(codedOut);
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


    Mode m;

    ofstream *ofs;
    OstreamOutputStream *zeroOut;
    CodedOutputStream *codedOut;

    ifstream *ifs;
    IstreamInputStream *zeroIn;
    CodedInputStream *codedIn;

    string filename;

};
