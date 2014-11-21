#pragma once

#include <snnlib/base.h>
#include <snnlib/protos/time_series.pb.h>
#include <snnlib/serialize/serialize.h>
#include <snnlib/serialize/proto_rw.h>

class TimeSeries : public Printable {
public:
    vector<double> data;

    TimeSeries() {}

    TimeSeries(const vector<double> &_data) : data(_data) {}

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
};


class LabeledTimeSeries  :  public Serializable {
public:
    string label;
    TimeSeries ts;

    LabeledTimeSeries() : Serializable(ELabeledTimeSeries) {}
    LabeledTimeSeries(const vector<double> &data, string _label) : Serializable(ELabeledTimeSeries), ts(data), label(_label) { }
    LabeledTimeSeries(const Protos::LabeledTimeSeries &m) : Serializable(ELabeledTimeSeries) {
        label = m.label();
        ts.deserialize(m.ts());
    }


    void print(std::ostream& str) const {
        str << "label: " << label << "\n";
        str << "data: " << ts;
    }

    // Serializable:
    LabeledTimeSeries(const LabeledTimeSeries &another) : Serializable(ELabeledTimeSeries), ts(another.ts), label(another.label) {
        copyFrom(another);
    }
    virtual Protos::LabeledTimeSeries* serialize() {
        Protos::LabeledTimeSeries *stat = getNew();
        Protos::TimeSeries *new_ts = stat->mutable_ts();
        for(auto it=ts.data.begin(); it != ts.data.end(); ++it) {
            new_ts->add_data(*it);
        }
        stat->set_label(label);
        return stat;
    }
    virtual void deserialize() {
        Protos::LabeledTimeSeries * m = castSerializableType<Protos::LabeledTimeSeries>(serialized_message);
        label = m->label();
        ts.deserialize(m->ts());
        //clean();
    }
    virtual Protos::LabeledTimeSeries* getNew(google::protobuf::Message* m = nullptr) {
        return getNewSerializedMessage<Protos::LabeledTimeSeries>(m);
    }
    // end Serializable

};



class LabeledTimeSeriesList  : public Serializable {
public:
    LabeledTimeSeriesList() : Serializable(ELabeledTimeSeriesList) {
    }
    LabeledTimeSeriesList(const string &fname) : Serializable(ELabeledTimeSeriesList) {
        deserializeFromFile(fname);
    }

    // Serializable:
    LabeledTimeSeriesList(const LabeledTimeSeriesList &l) : Serializable(ELabeledTimeSeriesList), ts(l.ts) {
        copyFrom(l);
    }
    virtual Protos::LabeledTimeSeriesList* serialize() {
        Protos::LabeledTimeSeriesList *l = getNew();
        for(auto it=ts.begin(); it != ts.end(); ++it) {
            Protos::LabeledTimeSeries* lts = l->add_list();
            *lts = *it->serialize();
        }
        return l;
    }

    virtual Protos::LabeledTimeSeriesList* getNew(google::protobuf::Message* m = nullptr) {
        return getNewSerializedMessage<Protos::LabeledTimeSeriesList>(m);
    }
    virtual void deserialize() {
        Protos::LabeledTimeSeriesList * m = castSerializableType<Protos::LabeledTimeSeriesList>(serialized_message);
        for(size_t li=0; li < m->list_size(); li++) {
            push_back(m->list(li));
        }
    }
    // end Serializable

    void deserializeFromFile(const string &fname) {
        ProtoRw prw(fname, ProtoRw::Read);
        prw.read(this);
    }
    void push_back(LabeledTimeSeries lst) {
        ts.push_back(lst);
    }

    void print(std::ostream& str) const {
        print_vector<LabeledTimeSeries>(ts, str, "\n");
    }
    size_t size() const {
        return ts.size();
    }

    vector<LabeledTimeSeries> ts;
};


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
         if(timeline.size()>0) {
            Tmax = timeline.back();
         }
         current_position = 0;
    }
    inline void pop_value() {
        data.pop_front();
    }
    inline const double& top_value() {
        return *data.begin();
    }
    inline size_t size() const {
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

