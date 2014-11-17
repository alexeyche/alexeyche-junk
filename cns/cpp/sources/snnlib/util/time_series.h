#pragma once

#include <snnlib/core.h>
#include <snnlib/protos/time_series.pb.h>
#include <snnlib/serialize/serialize.h>
#include <snnlib/serialize/proto_rw.h>

class TimeSeries { // : public Printable {
public:    
    vector<double> data;
    TimeSeries() {
        pos = 0;
    }
    void deserialize(const Protos::TimeSeries &m) {
        for(size_t i=0; i<m.data_size(); i++) {
            data.push_back( m.data(i) );
        }
    }
    void print(std::ostream& str) const {
        print_vector<double>(data, str, ", ");
        str << "\n";
    }
private:
    size_t pos;    
};


class LabeledTimeSeries { // : public Printable {
public:    
    string label;
    TimeSeries ts;

    void deserialize(const Protos::LabeledTimeSeries &m) {
        label = m.label();
        ts.deserialize(m.ts());
    }
    void print(std::ostream& str) const {
        str << "label: " << label << "\n";
        //str << "data: " << ts;
    }
};



class LabeledTimeSeriesList { // : public Printable {
public:    
    LabeledTimeSeriesList() : null_ret(0) {
        current_position.first = 0;
        current_position.second = 0;
    }
    LabeledTimeSeriesList(const string &fname) : null_ret(0) {
        deserializeFromFile(fname);
        current_position.first = 0;
        current_position.second = 0;
    }

    size_t size() const {
        return ts.size();
    }

    vector<LabeledTimeSeries> ts;
        
    const double &x pop_value() {
        const double &ret = ts[current_position.first].ts.data[current_position.second];
        current_position.second++;
        if(current_position.second == ts[current_position.first].ts.data.size() ) {
            current_position.first++;
            current_position.second = 0;
            if(current_position.first == ts.size()) {
                return null_ret;                    
            }
        }    
    }
    void deserializeFromFile(const string &fname) {
        ProtoRw prw(fname, ProtoRw::Read);
        vector<Protos::LabeledTimeSeries> v = prw.readAll<Protos::LabeledTimeSeries>();
        for(auto it=v.begin(); it!=v.end(); ++it) {
            LabeledTimeSeries o;
            o.deserialize(*it);
            ts.push_back(o);
        }
    }
    size_t getFullSampleLength() {
        size_t acc = 0;
        for(auto it=ts.begin(); it != ts.end(); ++it) {
            acc += it->ts.data.size();
            //cout << acc << "\n";
        }
        return acc;
    }

    void print(std::ostream& str) const {
        //print_vector<LabeledTimeSeries>(ts, str, "\n");
    }
private:    
    static double null_ret;
    pair<size_t,size_t> current_position;
};



