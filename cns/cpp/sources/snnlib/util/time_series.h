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


class LabeledTimeSeries  :  public Serializable<Protos::LabeledTimeSeries> {
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
    ProtoPack serialize() {
        Protos::LabeledTimeSeries *stat = getNewMessage();
        Protos::TimeSeries *new_ts = stat->mutable_ts();
        for(auto it=ts.data.begin(); it != ts.data.end(); ++it) {
            new_ts->add_data(*it);
        }
        stat->set_label(label);
        return ProtoPack({stat});
    }
    void deserialize() {
        Protos::LabeledTimeSeries * m = getSerializedMessage();
        label = m->label();
        ts.deserialize(m->ts());
    }
    // end Serializable

};



class LabeledTimeSeriesList  : public Serializable<Protos::LabeledTimeSeriesList> {
public:
    LabeledTimeSeriesList() : Serializable(ELabeledTimeSeriesList) {
    }

    // Serializable:
    ProtoPack serialize() {
        Protos::LabeledTimeSeriesList *l = getNewMessage();
        for(auto it=ts.begin(); it != ts.end(); ++it) {
            Protos::LabeledTimeSeries *lts = l->add_list();
            Protos::LabeledTimeSeries *lts_serial = castProtoMessage<Protos::LabeledTimeSeries>(it->serialize().front());
            *lts = *lts_serial;
        }
        return ProtoPack({l});
    }

    void deserialize() {
        Protos::LabeledTimeSeriesList *m = getSerializedMessage();
        for(size_t li=0; li < m->list_size(); li++) {
            push_back(m->list(li));
        }
    }
    // end Serializable

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

class PatternsTimeline : public Serializable<Protos::PatternsTimeline> {
public:
    PatternsTimeline() : Serializable(EPatternsTimeline), current_position(0) {}
    virtual ProtoPack serialize() {
        Protos::PatternsTimeline *l = getNewMessage();
        for(auto it=labels.begin(); it != labels.end(); ++it) {
            l->add_labels(*it);
        }
        for(auto it=labels_id_timeline.begin(); it != labels_id_timeline.end(); ++it) {
            l->add_labels_id_timeline(*it);
        }
        for(auto it=timeline.begin(); it != timeline.end(); ++it) {
            l->add_timeline(*it);
        }
        l->set_tmax(Tmax);
        l->set_gap_between_patterns(gapBetweenPatterns);
        l->set_dt(dt);
        return ProtoPack({l});
    }
    virtual void deserialize() {
        Protos::PatternsTimeline *m = getSerializedMessage();
        for(size_t i=0; i<m->labels_size(); i++) {
            labels.push_back(m->labels(i));
        }
        for(size_t i=0; i<m->labels_id_timeline_size(); i++) {
            labels_id_timeline.push_back(m->labels_id_timeline(i));
        }
        for(size_t i=0; i<m->timeline_size(); i++) {
            timeline.push_back(m->timeline(i));
        }
        Tmax = m->tmax();
        gapBetweenPatterns = m->gap_between_patterns();
        dt = m->dt();
    }
    const size_t& getCurrentClassId(const double &t) {
        while(current_position < timeline.size()) {
            if(t <= timeline[current_position]) {
                return labels_id_timeline[current_position];
            }
            current_position += 1;
        }
        cerr << "Trying to get current class for time bigger than Tmax: " << t << "\n";
        cout << "timeline: "; print_vector<double>(timeline,cout,","); cout << "\n";
        terminate();
    }
    const double& getCurrentPatternEnd(const double &t) {
        while(current_position < timeline.size()) {
            if(t <= timeline[current_position]) {
                return timeline[current_position];
            }
            current_position += 1;
        }
        cerr << "Trying to get current class for time bigger than Tmax: " << t << "\n";
        cout << "timeline: "; print_vector<double>(timeline,cout,","); cout << "\n";
        terminate();
    }

    void print(std::ostream& str) const {
        str << "labels: "; print_vector<string>(labels, str,","); str << "\n";
        str << "timeline: "; print_vector<double>(timeline,str,","); str << "\n";
        str << "labels id timeline: "; print_vector<size_t>(labels_id_timeline,str,","); str << "\n";
        str << "Tmax: " << Tmax << "\n";
    }
    void reset() {
        current_position = 0;
    }
    vector<string> getLabelsTimeline() {
        vector<string> labs;
        for(auto it = labels_id_timeline.begin(); it != labels_id_timeline.end(); ++it) {
            labs.push_back(labels[*it]);
        }
        return labs;
    }

    vector<string> labels;
    vector<size_t> labels_id_timeline;
    vector<double> timeline;
    double Tmax;
    double gapBetweenPatterns;
    double dt;

    size_t current_position;
};

typedef deque<double> ContData;

class ContLabeledTimeSeries : public Printable {
public:
    ContLabeledTimeSeries() {}
    ContLabeledTimeSeries(LabeledTimeSeriesList lst, const double &dt, const double &gapBetweenPatterns) {
        init(lst, dt, gapBetweenPatterns);
    }

    void init(LabeledTimeSeriesList lst, const double &dt, const double &gapBetweenPatterns) {
        double acc_time = 0;
        auto engine = std::default_random_engine{};
        std::random_shuffle(lst.ts.begin(), lst.ts.end());
        ptl.gapBetweenPatterns = gapBetweenPatterns;
        ptl.dt = dt;
        for(auto it=lst.ts.begin(); it != lst.ts.end(); ++it) {
            for(auto it_val=it->ts.data.begin(); it_val != it->ts.data.end(); ++it_val) {
                data.push_back(*it_val);
            }

            int pos = -1;
            if(ptl.labels.size()>0) {
                auto pos_it = find(ptl.labels.begin(), ptl.labels.end(), it->label);
                if(pos_it != ptl.labels.end()) {
                    pos = pos_it - ptl.labels.begin();
                }
            }

            if(pos<0) {
                ptl.labels.push_back(it->label);
                pos = ptl.labels.size()-1;
            }

            ptl.labels_id_timeline.push_back(pos);
            acc_time += dt*(double)it->ts.size() + ptl.gapBetweenPatterns;
            ptl.timeline.push_back(acc_time);
         }

         if(ptl.timeline.size()>0) {
            ptl.Tmax = ptl.timeline.back();
         }
         ptl.current_position = 0;
    }



    inline void pop_value() {
        data.pop_front();
    }
    inline const double& top_value() {
        return *data.begin();
    }
    bool dataIsReady(const double &t) {
        return (data.size()>0) && ((ptl.getCurrentPatternEnd(t) - ptl.gapBetweenPatterns) >= t);
    }
    inline size_t size() const {
        return data.size();
    }
    void print(std::ostream& str) const {
        str << "data: "; print_deque<double>(data, str,","); str << "\n";
        str << "patterns timeline: " << ptl << "\n";

    }
    void reset() {
        ptl.reset();
    }
    const double& getTmax() {
        return ptl.Tmax;
    }



    ContData data;
    PatternsTimeline ptl;
};


