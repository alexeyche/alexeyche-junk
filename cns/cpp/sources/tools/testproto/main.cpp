using namespace std;

#include <iostream>
#include <stdlib.h>
#include <fstream>
#include <sstream>

#include <google/protobuf/io/zero_copy_stream_impl.h>
#include <google/protobuf/io/coded_stream.h>


using namespace google::protobuf::io;

#include "time_series.pb.h"

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
            cerr << "Can't do ReadRaw of message size " << size << "\n";
            terminate();
        }
        //buffer[size] = '\0';
        if(!message.ParseFromArray(buffer, size)) {
            cerr << "Can't do parse of message size " << size << "\n";
            terminate();
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

void trim(string &str) {
    size_t endpos = str.find_last_not_of(" \t");
    if( string::npos != endpos )
    {
        str = str.substr( 0, endpos+1 );
    }
    size_t startpos = str.find_first_not_of(" \t");
    if( string::npos != startpos )
    {
        str = str.substr( startpos );
    }
}

vector<string> split_into(const string &s, char delim, vector<string> &elems) {
    stringstream ss(s);
    string item;
    while (getline(ss, item, delim)) {
        elems.push_back(item);
    }
    return elems;
}


vector<string> split(const string &s, char delim) {
    vector<string> elems;
    split_into(s, delim, elems);
    return elems;
}

Protos::LabeledTimeSeries doubleVectorToLabeledTimeSeries(string label, const vector<double> &data) {
    Protos::LabeledTimeSeries lts;
    Protos::TimeSeries *ts = new Protos::TimeSeries();
    lts.set_label(label);
    lts.set_allocated_ts(ts);
    for(auto it=data.cbegin(); it != data.cend(); ++it) {
        ts->add_data(*it);
    }
    return lts;
}

Protos::LabeledTimeSeries convertUcrTimeSeriesLine(const string &line) {
    vector<string> els = split(line, ' ');
    assert(els.size() > 0);
    
    string lab;
    vector<double> ts_data;
    for(size_t i=0; i<els.size(); i++) {
        trim(els[i]);
        if(!els[i].empty()) {
            if(lab.empty()) {
                std::ostringstream lab_format;
                lab_format << stoi(els[i]);
                lab = lab_format.str();
                continue;
            }
            ts_data.push_back(stof(els[i]));
        }
    }
    
    Protos::LabeledTimeSeries ts = doubleVectorToLabeledTimeSeries(lab, ts_data);
    return ts;
}



int main(int argc, char **argv) {
	{
		ProtoRw prw(argv[2], ProtoRw::Write);
	    ifstream ucr_ts_file(argv[1]);
		string line;
	    int i =0;
	    while ( getline (ucr_ts_file,line) ) {
	        Protos::LabeledTimeSeries ts = convertUcrTimeSeriesLine(line);
	        prw.write<Protos::LabeledTimeSeries>(ts);
	    }            
	}
	{
		ProtoRw prw(argv[2], ProtoRw::Read);
	    vector<Protos::LabeledTimeSeries> v = prw.readAll<Protos::LabeledTimeSeries>();
        for(auto it=v.begin(); it!=v.end(); ++it) {
        	cout << it->DebugString();
        }
	}
}