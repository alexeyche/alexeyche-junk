#pragma once

#include <snnlib/protos/time_series.pb.h>
#include <google/protobuf/io/zero_copy_stream_impl.h>
#include <google/protobuf/io/coded_stream.h>

#include <fcntl.h>

using namespace google::protobuf::io;

Protos::LabeledTimeSeries doubleVectorToLabeledTimeSeries(string label, const vector<double> &data) {
    Protos::LabeledTimeSeries lts;
    Protos::TimeSeries *ts = new Protos::TimeSeries();
    lts.set_label(label);
    lts.set_allocated_ts(ts);
    for(auto it=data.cbegin(); it != data.cend(); ++it) {
        ts->add_data(*it);
    }
    return lts;
}


