#pragma once

#include <snnlib/protos/stat.pb.h>
#include <snnlib/serialize/serialize.h>


class Neuron;

class NeuronStat : public Serializable<Protos::NeuronStat> {
protected:
    NeuronStat() : Serializable<Protos::NeuronStat>(ENeuronStat) { }
    friend class Factory;
public:
    NeuronStat(Neuron *n);



    void collect(Neuron *n);

    ProtoPack serialize();

    virtual void deserialize() {
        Protos::NeuronStat * m = getSerializedMessage();
        for(size_t i=0; i<m->p_size(); i++) {
            p.push_back(m->p(i));
        }
        for(size_t i=0; i<m->u_size(); i++) {
            u.push_back(m->u(i));
        }
        for(size_t i=0; i<m->m_size(); i++) {
            M.push_back(m->m(i));
        }
        for(size_t i=0; i<m->syns_size(); i++) {
            Protos::NeuronStat::SynStat syn_m = m->syns(i);

            vector<double> x_v;
            vector<double> w_v;
            assert(syn_m.x_size() == syn_m.w_size());
            for(size_t j=0; j<syn_m.x_size(); j++) {
                x_v.push_back(syn_m.x(j));
                w_v.push_back(syn_m.w(j));
            }
            syns.push_back(x_v);
            w.push_back(w_v);
        }
    }

    void print(std::ostream& str) const {
    }

    vector<vector<double>> syns;
    vector<vector<double>> w;
    vector<double> p;
    vector<double> u;
    vector<double> M;
};

