
#include "serialize.h"


#include <snnlib/util/time_series.h>
#include <snnlib/neurons/neuron.h>
#include <snnlib/neurons/adex_neuron.h>
#include <snnlib/util/spikes_list.h>



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
