
#include "neuron.h"




Neuron::Neuron(const ConstObj *_c, const RuntimeGlobals *_glob_c, double _axon_delay) {
    init(_c, _glob_c, _axon_delay);
}
Neuron::Neuron(const Neuron &another) : Serializable(ENeuron) {
    copyFrom(another);
}

// init
void Neuron::init(const ConstObj *_c, const RuntimeGlobals *_glob_c, double _axon_delay) {
    Serializable::init(ENeuron);
    id = global_neuron_index++;
    bc = _c;
    glob_c = _glob_c;

    act = nullptr; lrule = nullptr; tc = nullptr;

    y = 0.0;
    p = 0.0;
    fired = 0;

    collectStatistics = false;
    stat = nullptr;
    axon_delay = _axon_delay;
}

void Neuron::setActFunc(ActFunc *_act) {
    act = _act;
}
void Neuron::setLearningRule(LearningRule *_lrule) {
    lrule = _lrule;
}
void Neuron::setTuningCurve(TuningCurve *_tc) {
    tc = _tc;
}
void Neuron::addSynapse(Synapse *s) {
    syns.push_back(s);
    if(collectStatistics) {
        stat->syns.push_back(vector<double>());
    }
}

/////////////////////////////////////////////////////////////////////////////
// serialize
void Neuron::saveModel(ProtoRw &rw) {
    Protos::Neuron n_ser;
    n_ser.set_axon_delay(axon_delay);
    n_ser.set_id(id);
    n_ser.set_num_of_synapses(syns.size());
    rw.writeMessage(&n_ser);

    for(auto it=syns.begin(); it != syns.end(); ++it) {
        it->save(rw);
    }
}

void Neuron::loadModel(ProtoRw &rw) {
    Protos::Neuron n_ser;
    rw.readMessage(&n_ser);
    id = n_ser->id();
    axon_delay = n_ser->axon_delay();
    for(size_t syn_i=0; syn_i<num_of_synapses; syn_i++) {

    }
}

////////////////////////////////////////////////////////////////////////////
// stat funcs
void Neuron::saveStat(SerialPack &p) {
    SerialFamily f({stat});
    lrule->saveStat(f);
    p.push_back(f);
}

void Neuron::enableCollectStatistics() {
    collectStatistics = true;
    stat = Factory::inst().registerObj<NeuronStat>(new NeuronStat(this));
}

// print
void Neuron::print(std::ostream& str) const {
    str << "Neuron(" << id << ")\n";
    str << "\ty == " << y << ", axon_delay: " << axon_delay << ", synapses\n";
    for(auto it=syns.begin(); it != syns.end(); ++it) {
        Synapse *s = *it;
        str << *s << ", ";
    }
    str << "\n";
}
