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

template <typename V>
void pop_front(V & v)
{
    assert(!v.empty());
    v.erase(v.begin());
}

class ContLabeledTimeSeries : public Printable {
public:
    ContLabeledTimeSeries() {}
    ContLabeledTimeSeries(LabeledTimeSeriesList &lst, const double &dt) {
        init(lst, dt);
    }
    
    void init(LabeledTimeSeriesList &lst, const double &dt) {
        double acc_time = 0;
        for(auto it=lst.ts.begin(); it != lst.ts.end(); ++it) {
            for(auto it_val=it->ts.data.begin(); it_val != it->ts.data.end(); ++it_val) {
                data.push_back(*it_val);                
            }
            labels.push_back(it->label);
            acc_time += dt*(double)it->ts.size();
            timeline.push_back(acc_time);
         }
         Tmax = timeline.back();
         current_position = 0;
    }
    const double & pop_value() {
        const double &x = *data.begin();
        data.pop_front();
        return x;
    }
    size_t size() const {
        return data.size();
    }    
    void print(std::ostream& str) const {
        str << "data: "; print_deque<double>(data, str,","); str << "\n";
        str << "labels: "; print_vector<string>(labels, str,","); str << "\n";
        str << "timeline: "; print_vector<double>(timeline,str,","); str << "\n";
        str << "Tmax: " << Tmax << "\n";
    }

    size_t current_position;
    double Tmax;

    deque<double> data;
    vector<string> labels;
    vector<double> timeline;    
};

