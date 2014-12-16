#pragma once

#include <snnlib/protos/spikes_list.pb.h>

#include <snnlib/util/time_series.h>
#include <snnlib/serialize/serialize.h>

class SpikesList: public Serializable<Protos::SpikesList> {
public:
    SpikesList() : Serializable<Protos::SpikesList>(ESpikesList), sp_list(nullptr) {}
    SpikesList(size_t _N) : Serializable<Protos::SpikesList>(ESpikesList), N(_N) {
        init(N);
    }
    SpikesList(const SpikesList &another) : Serializable(ESpikesList)  {
        Serializable<Protos::SpikesList>::copyFrom(another);
        init(another.N);
        for(size_t ni=0; ni<N; ni++) {
            sp_list[ni] = another.sp_list[ni];
        }
    }
    virtual ProtoPack serialize() {
        Protos::SpikesList *l = getNewMessage();
        for(size_t ni=0; ni<N; ni++) {
            Protos::SpikesSequence* seq = l->add_spikes_list();
            for(auto it=sp_list[ni].begin(); it != sp_list[ni].end(); ++it) {
                seq->add_seq(*it);
            }
        }
        return ProtoPack({l});
    }


    virtual void deserialize() {
        Protos::SpikesList * m = getSerializedMessage();
        init(m->spikes_list_size());
        for(size_t ni=0; ni<N; ni++) {
            Protos::SpikesSequence s = m->spikes_list(ni);
            for(size_t el_i=0; el_i<s.seq_size(); el_i++) {
                sp_list[ni].push_back(s.seq(el_i));
            }
        }
    }

    void init(size_t _N)  {
        N = _N;
        sp_list = new vector<double>[N];
    }
    vector<double>& operator[](size_t ni) {
        return sp_list[ni];
    }
    const vector<double>& operator[](size_t ni) const {
        return sp_list[ni];
    }
    void print(std::ostream& str) const {
        str << N << "spikes lists:\n";
        for(size_t ni=0; ni<N; ni++) {
            str << "#" << ni << ": ";
            print_vector<double>(sp_list[ni],str,",");
        }
    }
    ~SpikesList() {
        if(sp_list) {
            delete []sp_list;
        }
    }
    double getMaxSpikeTime() {
        double max_t = 0;
        for(size_t ni=0; ni<N; ni++) {
            if(sp_list[ni].size()>0) {
                auto it = std::max_element(std::begin(sp_list[ni]), std::end(sp_list[ni]));
                if(*it > max_t) {
                    max_t = *it;
                }
            }
        }
        return max_t;
    }


    size_t N;
    vector<double> *sp_list;
};

class LabeledSpikesList: public Serializable<Protos::LabeledSpikesList> {
private:
    friend class Factory;
    LabeledSpikesList() : Serializable(ELabeledSpikesList) {}
public:
    LabeledSpikesList(PatternsTimeline &_ptl, SpikesList &_sl) : Serializable(ELabeledSpikesList),
        sl(_sl), ptl(_ptl) {}


    virtual ProtoPack serialize() {
        Protos::LabeledSpikesList *l = getNewMessage();

        Protos::SpikesList *sl_m = copyProtoMessage<Protos::SpikesList>(sl.serialize().front());
        Protos::PatternsTimeline *ptl_m = copyProtoMessage<Protos::PatternsTimeline>(ptl.serialize().front());

        l->set_allocated_sl(sl_m);
        l->set_allocated_ptl(ptl_m);
        return ProtoPack({l});
    }


    virtual void deserialize() {
        Protos::LabeledSpikesList *m = getSerializedMessage();
        sl.deserializeFromAllocated(m->mutable_sl());
        ptl.deserializeFromAllocated(m->mutable_ptl());
    }

    void print(std::ostream& str) const {
        cout << "SpikesList: " << sl;
        cout << "PatternsTimeline: " << ptl;
    }
    SpikesList sl;
    PatternsTimeline ptl;
};
