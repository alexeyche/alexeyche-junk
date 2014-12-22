
#include "neuron.h"




Neuron::Neuron(const ConstObj *_c, size_t _local_id, const RuntimeGlobals *_glob_c, double _axon_delay) : Serializable(ENeuron) {
    init(_c, _local_id, _glob_c, _axon_delay);
}

// init
void Neuron::init(const ConstObj *_c, size_t _local_id, const RuntimeGlobals *_glob_c, double _axon_delay) {
    id = global_neuron_index++;
    bc = _c;
    glob_c = _glob_c;

    lrule = nullptr;
    tc = nullptr;
    rmod = nullptr;
    LearningRule::provideDefaultRuntime(lrule_rt);
    RewardModulation::provideDefaultRuntime(rmod_rt);
    TuningCurve::provideDefaultRuntime(tc_rt);


    reset();

    collectStatistics = false;
    stat = nullptr;
    axon_delay = _axon_delay;


    local_id = _local_id;
}

void Neuron::setActFunc(ActFunc *act) {
    act->provideRuntime(act_rt);
}
void Neuron::setLearningRule(LearningRule *_lrule) {
    lrule = _lrule;
    lrule->provideRuntime(lrule_rt);
}
void Neuron::setTuningCurve(TuningCurve *_tc) {
    tc = _tc;
    tc->provideRuntime(tc_rt);
}
void Neuron::setRewardModulation(RewardModulation *_rmod) {
    rmod = _rmod;
    rmod->provideRuntime(rmod_rt);
}

void Neuron::addSynapse(Synapse *s) {
    syns.resize(syns.size() + 1);
    addSynapseAtPos(s, syns.size()-1);
}
void Neuron::addSynapseAtPos(Synapse *s, const size_t &pos_i) {
    if(pos_i>syns.size()) {
        cerr << "Can't add synapse at that position\n";
        terminate();
    }
    if(collectStatistics) {
        stat->addSynapse(s);
    }
    if(lrule) lrule->addSynapse(s);
    syns[pos_i] = s;
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
    return ProtoPack({n_ser});
}
void Neuron::deserialize() {
    Protos::Neuron *mess = getSerializedMessage();
    id = mess->id();
    axon_delay = mess->axon_delay();
    if(syns.size()>0) {
        syns.clear();
    }
    syns.resize(mess->num_of_synapses());
}

void Neuron::saveModel(ProtoRw &rw) {
    rw.write(this);
    if(lrule) {
        rw.write(lrule);
    }
    for(size_t syn_i=0; syn_i<syns.size(); syn_i++) {
    //cout << *syns[syn_i] << "\n";
        rw.write(syns[syn_i]);
    }
}

void Neuron::loadModel(ProtoRw &rw) {
    rw.readAllocated(this);
    if(lrule) {
        rw.readAllocated(lrule);
    }
    for(size_t syn_i=0; syn_i<syns.size(); syn_i++) {
        addSynapseAtPos(rw.read()->castSerializable<Synapse>(), syn_i);
    }
}

////////////////////////////////////////////////////////////////////////////
// stat funcs
void Neuron::saveStat(SerialPack &p) {
    p.push_back(stat);
    if(stat->mode == NeuronStat::Full) {
        lrule->saveStat(p);
    }
}

void Neuron::enableCollectStatistics() {
    collectStatistics = true;
    stat = Factory::inst().registerObj<NeuronStat>(new NeuronStat(this, NeuronStat::Full));
    lrule->enableCollectStatistics();
}
void Neuron::enableCollectProbStatistics() {
    collectStatistics = true;
    stat = Factory::inst().registerObj<NeuronStat>(new NeuronStat(this, NeuronStat::PStat));
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
