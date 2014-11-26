
#include "serialize.h"


#include <snnlib/util/time_series.h>
#include <snnlib/neurons/neuron.h>
#include <snnlib/neurons/adex_neuron.h>
#include <snnlib/util/spikes_list.h>

SerializableFactory& SerializableFactory::inst() {
    static SerializableFactory _inst;
    return _inst;
}

Serializable* SerializableFactory::create(const string &name) {
    Serializable *s = nullptr;
    if(name == "AdExNeuronStat") {
        s = new AdExNeuronStat();
    } else
    if(name == "NeuronStat") {
        s = new NeuronStat();
    } else
    if(name == "Neuron") {
        cerr << "Can't create such instance: " << name << "\n";
        terminate();
    } else
    if(name == "SpikesList") {
        s = new SpikesList();
    } else
    if(name == "LabeledTimeSeries") {
        s = new LabeledTimeSeries();
    } else
    if(name == "LabeledTimeSeriesList") {
        s = new LabeledTimeSeriesList();
    } else {
        cerr << "Unkown Serializable name " << name << "\n";
        terminate();
    }
    objects.push_back(s);
    return s;
}





// methods


Protos::LabeledTimeSeries doubleVectorToLabeledTimeSeries(string label, const vector<double> &data) {
    Protos::LabeledTimeSeries lts;
    Protos::TimeSeries *ts = lts.mutable_ts();
    for(auto it=data.cbegin(); it != data.cend(); ++it) {
        ts->add_data(*it);
    }
    lts.set_label(label);
    return lts;
}
