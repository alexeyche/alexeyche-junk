#pragma once

#include <snnlib/protos/spikes_list.pb.h>

#include <snnlib/serialize/serialize.h>

class SpikesList: public Serializable {
public:
    SpikesList() : Serializable(ESpikesList), sp_list(nullptr) {}
    SpikesList(size_t _N) : Serializable(ESpikesList), N(_N) {
        init(N);
    }
    SpikesList(const SpikesList &another) : Serializable(ESpikesList)  {
        copyFrom(another);
        init(another.N);
        for(size_t ni=0; ni<N; ni++) {
            sp_list[ni] = another.sp_list[ni];
        }
    }
    virtual Protos::SpikesList* serialize() {
        Protos::SpikesList *l = getNew();
        for(size_t ni=0; ni<N; ni++) {
            Protos::SpikesSequence* seq = l->add_spikes_list();
            for(auto it=sp_list[ni].begin(); it != sp_list[ni].end(); ++it) {
                seq->add_seq(*it);
            }
        }
        return l;
    }

    virtual Protos::SpikesList* getNew(google::protobuf::Message* m = nullptr) {
        return getNewSerializedMessage<Protos::SpikesList>(m);
    }

    virtual void deserialize() {
        Protos::SpikesList * m = castSerializableType<Protos::SpikesList>(serialized_message);
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

    size_t N;
    vector<double> *sp_list;
};
