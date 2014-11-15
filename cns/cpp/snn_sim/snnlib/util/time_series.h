#pragma once

#include <snnlib/core.h>
#include <snnlib/protos/time_series.pb.h>
#include <snnlib/serialize/serialize.h>
#include <snnlib/serialize/proto_rw.h>

class TimeSeries : public Printable {
public:    
    vector<double> data;

    void deserialize(const Protos::TimeSeries &m) {
        for(size_t i=0; i<m.data_size(); i++) {
            data.push_back( m.data(i) );
        }
    }
    void print(std::ostream& str) const {
        print_vector<double>(data, str, ", ");
        str << "\n";
    }
};


class LabeledTimeSeries : public Printable {
public:    
    string label;
    TimeSeries ts;

    void deserialize(const Protos::LabeledTimeSeries &m) {
        label = m.label();
        ts.deserialize(m.ts());
    }
    void print(std::ostream& str) const {
        str << "label: " << label << "\n";
        str << "data: " << ts;
    }
};


class LabeledTimeSeriesList : public Printable {
public:    
    LabeledTimeSeriesList(const string &fname) {
        deserializeFromFile(fname);
    }

    vector<LabeledTimeSeries> ts;

    void deserializeFromFile(const string &fname) {
        ProtoRw prw(fname, ProtoRw::Read);
        vector<Protos::LabeledTimeSeries> v = prw.readAll<Protos::LabeledTimeSeries>();
        for(auto it=v.begin(); it!=v.end(); ++it) {
            LabeledTimeSeries o;
            o.deserialize(*it);
            ts.push_back(o);
        }
    }
    void print(std::ostream& str) const {
        print_vector<LabeledTimeSeries>(ts, str, "\n");
    }
};



