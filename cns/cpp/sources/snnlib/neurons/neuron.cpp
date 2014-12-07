
#include "neuron.h"




Neuron::Neuron(const ConstObj *_c, const RuntimeGlobals *_glob_c, double _axon_delay) : Serializable(ENeuron) {
    init(_c, _glob_c, _axon_delay);
}

// init
void Neuron::init(const ConstObj *_c, const RuntimeGlobals *_glob_c, double _axon_delay) {
    //Serializable::init(ENeuron);
    id = global_neuron_index++;
    bc = _c;
    glob_c = _glob_c;

    act = nullptr; lrule = nullptr; tc = nullptr;

    reset();
    weight_factor = 1.0;

    collectStatistics = false;
    stat = nullptr;
    axon_delay = _axon_delay;
    rmod = nullptr;
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
void Neuron::setRewardModulation(RewardModulation *_rmod) {
    rmod = _rmod;
}
void Neuron::addSynapse(Synapse *s) {
    syns.push_back(s);
    if(collectStatistics) {
        stat->syns.push_back(vector<double>());
    }
    lrule->addSynapse(s);
}

void Neuron::reset() {
    y = 0.0;
    p = 0.0;
    gr = 0.0;
    M = 0.0;
    fired = 0;
    for(auto it=syns.begin(); it != syns.end(); ++it) {
        Synapse *s = *it;
        s->reset();
    }
    active_synapses.clear();
}

/////////////////////////////////////////////////////////////////////////////
// serialize

ProtoPack Neuron::serialize() {
    Protos::Neuron *n_ser = getNewMessage();
    n_ser->set_axon_delay(axon_delay);
    n_ser->set_id(id);
    n_ser->set_num_of_synapses(syns.size());
    n_ser->set_weight_factor(weight_factor);
    return ProtoPack({n_ser});
}
void Neuron::deserialize() {
    Protos::Neuron *mess = getSerializedMessage();
    id = mess->id();
    axon_delay = mess->axon_delay();
    weight_factor = mess->weight_factor();
    if(syns.size()>0) {
        syns.clear();
    }
    syns.resize(mess->num_of_synapses());
}

void Neuron::saveModel(ProtoRw &rw) {
    rw.write(this);
    for(size_t syn_i=0; syn_i<syns.size(); syn_i++) {
        rw.write(syns[syn_i]);
    }
    if(!lrule->isBlank()) {
        rw.write(lrule);
    }
}

void Neuron::loadModel(ProtoRw &rw) {
    rw.readAllocated(this);
    for(size_t syn_i=0; syn_i<syns.size(); syn_i++) {
        syns[syn_i] = rw.read()->castSerializable<Synapse>();
    }
    if(!lrule->isBlank()) {
        rw.readAllocated(lrule);
    }
}

////////////////////////////////////////////////////////////////////////////
// stat funcs
void Neuron::saveStat(SerialPack &p) {
    p.push_back(stat);
    lrule->saveStat(p);
}

void Neuron::enableCollectStatistics() {
    collectStatistics = true;
    stat = Factory::inst().registerObj<NeuronStat>(new NeuronStat(this));
    lrule->enableCollectStatistics();
}

// print
void Neuron::print(std::ostream& str) const {
    str << "Neuron(" << id << ")\n";
    str << "\ty == " << y << ", axon_delay: " << axon_delay << ", synapses\n";
    for(auto it=syns.begin(); it != syns.end(); ++it) {
        Synapse *s = *it;
        str << *s << ", ";
    }
    str << "lrule: " << lrule->getName() << "\n";
    str << "\n";
}
