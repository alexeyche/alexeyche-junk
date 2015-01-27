#pragma once


#include "synapse.h"

// static size_t synglob_id = 0;

class DynamicSynapse : public Synapse {
protected:
    DynamicSynapse() {}
    friend class Factory;
public:
    DynamicSynapse(const ConstObj *_c, size_t _id_pre, double _w, double _dendrite_delay) {
        init(_c, _id_pre, _w, _dendrite_delay);
    }
    ~DynamicSynapse() {
        // if(id == 2090) {
        //     JsonBox::Value out;
        //     out["x"] = x_stat;
        //     out["rec_res"] = rec_res_stat;
        //     out["inact_res"] = inact_res_stat;
        //     out.writeToFile("/var/tmp/dynamic_synapse_stat.json");
        // }

    }
    void init(const ConstObj *_c, size_t _id_pre, double _w, double _dendrite_delay) {
        Synapse::init(_c, _id_pre, _w, _dendrite_delay);
        Serializable::init(EDynamicSynapse);
        c = castType<DynamicSynapseC>(_c);
        // id = synglob_id;
        // synglob_id++;
    }

    void calculateDynamics() {
        // if(id==2090) {
        //     x_stat.push_back(x);
        //     rec_res_stat.push_back(rec_res);
        //     inact_res_stat.push_back(inact_res);
        // }
        rec_res   += inact_res/c->tau_rec; // x
        x         += -x/c->tau_in;  // y
        inact_res += x/c->tau_in - inact_res/c->tau_rec;
    }
    void reset() {
        rec_res = 1.0;
        x = 0.0;
        inact_res = 0.0;
    }
    void propagateSpike() {
        double use_of_res = c->u_se * rec_res; 
        rec_res -= use_of_res;
        x += use_of_res;
    }
    double getCurrent() {
        return x * c->a_se;
    }
    void provideRuntime(SynapseRuntime &srt) {
        srt.calculateDynamics = MakeDelegate(this, &DynamicSynapse::calculateDynamics);
        srt.propagateSpike = MakeDelegate(this, &DynamicSynapse::propagateSpike);
    }

    
    void print(std::ostream& str) const {
        str << "DynamicSynapse(id_pre: " << Synapse::id_pre << ", x:" << Synapse::x << ", w: " << Synapse::w << ", dendrite_delay: " << Synapse::dendrite_delay << ")";
    }
    double rec_res; // x
    double inact_res; // z
    
    // size_t id;
    // JsonBox::Array x_stat;
    // JsonBox::Array rec_res_stat;
    // JsonBox::Array inact_res_stat;

    const DynamicSynapseC *c;
};