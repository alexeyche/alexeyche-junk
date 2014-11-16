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
            if(!ifs->is_open()) {
                cerr << "Failed to open file " << f << " for read\n";
                terminate(); 
            }
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
            ifs->close();

            delete codedIn;
            delete zeroIn;
            
            delete ifs;            
            
        } else
        if(m == Write) {
            ofs->close();
            
            delete codedOut;
            delete zeroOut;
            
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
        while(!ifs->eof()) {
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
        google::protobuf::uint32 size = message.ByteSize();
        char buffer[size]; 
        if(!message.SerializeToArray(buffer, size)) {
            cerr << "Failed to serialize message: \n" << message.DebugString();
            terminate();
        }
        google::protobuf::uint32 magic = 1234;
        codedOut->WriteLittleEndian32(magic);
        codedOut->WriteVarint32(size);
        codedOut->WriteRaw(buffer, size);
    }
    bool readMessage(::google::protobuf::Message &message) {
        google::protobuf::uint32 magic;
        if(!codedIn->ReadLittleEndian32(&magic)) {
            return false;
        }
        
        google::protobuf::uint32 size;
        if (!codedIn->ReadVarint32(&size)) { 
            return false;
        }
        char buffer[size];
        
        cout << size << "\n";
        if(!codedIn->ReadRaw(buffer, size)) {
            //cerr << "Can't do ReadRaw of message size " << size << "\n";
            //terminate();
        }
        //buffer[size] = '\0';
        if(!message.ParseFromArray(buffer, size)) {
            //cerr << "Can't do parse of message size " << size << "\n";
            //terminate();
        }
        cout << message.DebugString();
        return true;
    }




    Mode m;
    ofstream *ofs;
    OstreamOutputStream *zeroOut;
    CodedOutputStream *codedOut;

    ifstream *ifs;
    IstreamInputStream *zeroIn;
    CodedInputStream *codedIn;

};
