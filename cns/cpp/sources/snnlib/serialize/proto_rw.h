#pragma once

#include <snnlib/protos/time_series.pb.h>
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
    ProtoRw(const string &f, Mode _m) : m(_m) {
        if(m == Read) {
            fd = open(f.c_str(), O_RDONLY);
            if (fd == -1) {
                perror("Error while opening file");
                terminate();
            }

            ifs_g = new google::protobuf::io::FileInputStream(fd);
            codedIn = new CodedInputStream(ifs_g);
        } else
        if(m == Write) {
            ofs = new ofstream(f, ios::out | ios::trunc | ios::binary);
        }
    }
    ~ProtoRw() {
        if(m == Read) {
            delete ifs_g;
            delete codedIn;

            close(fd);
        } else
        if(m == Write) {
            ofs->close();
            delete ofs;
        }

    }
    template <typename T>
    bool read(T &el) {
        CHECK_MODE(Read);
        if(!readMessage(el)) {
            return false;
        }
        return true;
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
        while(true) {
            T el;
            if(!read<T>(el)) {
                break; // we are at end
            }
            v.push_back(el);
        }
        return v;
    }

private:
    void writeMessage(::google::protobuf::Message &message) {
        zeroOut = new OstreamOutputStream(ofs);
        codedOut = new CodedOutputStream(zeroOut);

        google::protobuf::uint32 size = message.ByteSize();

        codedOut->WriteVarint32(size);
        message.SerializeToCodedStream(codedOut);

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

};
