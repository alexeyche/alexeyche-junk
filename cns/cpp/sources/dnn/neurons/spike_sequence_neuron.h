#pragma once


#include "spike_neuron.h"
#include <dnn/protos/generated.pb.h>
#include <dnn/io/serialize.h>

namespace dnn {


/*@GENERATE_PROTO@*/
struct SpikeSequenceNeuronC : public Serializable<Protos::SpikeSequenceNeuronC> {
    SpikeSequenceNeuronC() 
    : 
      dt(1.0)
    {}

    void serial_process() {
        begin() << "dt: " << dt << Self::end;
    }

    double dt;
};


/*@GENERATE_PROTO@*/
struct SpikeSequenceNeuronState : public Serializable<Protos::SpikeSequenceNeuronState>  {
    SpikeSequenceNeuronState() 
    : p(0.0), index(0)
    {}

    void serial_process() {
        begin() << "p: " << p << ", " << "index: " << index << Self::end;
    }    
    double p;
    size_t index;
};


class SpikeSequenceNeuron : public SpikeNeuron<SpikeSequenceNeuronC, SpikeSequenceNeuronState> {
public:
    const string name() const {
        return "SpikeSequenceNeuron";
    }

    void reset() {
        if(input.isSet()) {
            throw dnnException() << "Got current inputs in SpikeSequenceNeuron. Config is errors prone\n";
        }
        if(lrule.isSet()) {
            throw dnnException() << "Got learning rule in SpikeSequenceNeuron. Config is errors prone\n";
        }
        if(act_f.isSet()) {
            throw dnnException() << "Got activation function in SpikeSequenceNeuron. Config is errors prone\n";
        }
        s.p = 0.0;
        s.index = 0;
    }

    void calculateDynamics(const Time& t, const double &Iinput, const double &Isyn) {
        if(s.index>=seq.size()) return;
        const double &spike_time = seq[s.index];
        if((spike_time>=t.t)&&(spike_time<(t.t+t.dt))) {
            s.index++;
            setFired(true);
        }
    }
   
    const double& getFiringProbability() {
        return s.p;
    }
    void setAsInput(SerializableBase *b) {
        SpikesList *sl = as<SpikesList>(b);
        size_t id = localId();
        if(id>=sl->size()) {
            throw dnnException() << "Got input spike sequence less than neuron count\n";
        }
        seq = sl->seq[id].values;
    }

    double getSimDuration() {
        if(input.isSet()) {
            throw dnnException() << "Got current inputs in SpikeSequenceNeuron. Config is errors prone\n";
        }
        if(seq.size() == 0) return 0.0;
        return seq.back() * c.dt;
    }
private:    
    vector<double> seq;
};

}
