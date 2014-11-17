#pragma once

#include <snnlib/core.h>
#include <snnlib/protos/time_series.pb.h>
#include <snnlib/serialize/serialize.h>
#include <snnlib/serialize/proto_rw.h>

class TimeSeries : public Printable {
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
    void push_back(const double &x) {
        data.push_back(x);
    }
    size_t size() {
        return data.size();
    }
private:
    size_t pos;    
};


class LabeledTimeSeries  : public Printable {
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



class LabeledTimeSeriesList  : public Printable {
public:    
    LabeledTimeSeriesList() {
    }
    LabeledTimeSeriesList(const string &fname) {
        deserializeFromFile(fname);
    }

    size_t size() const {
        return ts.size();
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
    size_t getFullSampleLength() {
        size_t acc = 0;
        for(auto it=ts.begin(); it != ts.end(); ++it) {
            acc += it->ts.data.size();
        }
        return acc;
    }

    void print(std::ostream& str) const {
        print_vector<LabeledTimeSeries>(ts, str, "\n");
    }
private:    
    static double null_ret;
};

class ContLabeledTimeSeries : public Printable {
public:
    ContLabeledTimeSeries() {}
    ContLabeledTimeSeries(LabeledTimeSeriesList &lst, const double &dt) {
        init(lst, dt);
    }
    
    void init(LabeledTimeSeriesList &lst, const double &dt) {
        for(auto it=lst.ts.begin(); it != lst.ts.end(); ++it) {
            for(auto it_val=it->ts.data.begin(); it_val != it->ts.data.end(); ++it_val) {
                ts.push_back(*it_val);                
            }
            labels.push_back(it->label);
            timeline.push_back(dt*(double)it->ts.size());
         }
         Tmax = timeline.back();

    }
    const double & pop_value() {
        ts.data.back();            
    }
    size_t size() const {
        return ts.data.size();
    }    
    void print(std::ostream& str) const {
    }
    double Tmax;

    TimeSeries ts;
    vector<string> labels;
    vector<double> timeline;    
};

