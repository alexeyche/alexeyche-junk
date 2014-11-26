
#include "neuron.h"




Neuron::Neuron(const ConstObj *_c, const RuntimeGlobals *_glob_c, double _axon_delay) {
    init(_c, _glob_c, _axon_delay);
}
Neuron::Neuron(const Neuron &another) : Serializable(ENeuron) {
    copyFrom(another);
}

// init
void Neuron::init(const ConstObj *_c, const RuntimeGlobals *_glob_c, double _axon_delay) {
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
    Serializable::init(ENeuron);
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
Protos::Neuron* Neuron::getNew(google::protobuf::Message* m = nullptr) {
    return getNewSerializedMessage<Protos::Neuron>(m);
}
Protos::Neuron *Neuron::serialize() {
    Protos::Neuron *n_ser = getNew();
    for(auto it=syns.begin(); it != syns.end(); ++it) {
        Synapse *s = *it;
        Protos::Neuron::Syn *syn = n_ser->add_syns();
        syn->set_w(s->w);
        syn->set_id_pre(s->id_pre);
        syn->set_dendrite_delay(s->dendrite_delay);
    }
    n_ser->set_axon_delay(axon_delay);
    n_ser->set_id(id);
    return n_ser;
}
void Neuron::deserialize() {
    Protos::Neuron *n_ser = castSerializableType<Protos::Neuron>(serialized_message);
    Synapse *s = Factory::inst().createSynapse(syns[0]->c->getName(), glob_c->C(), 0, 0, 0);
}
void Neuron::readModel(ProtoRw &rw) {
    //rw.read(this);
    //rw.read(lrule);
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
