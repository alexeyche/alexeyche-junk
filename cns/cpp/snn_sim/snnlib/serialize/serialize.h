#pragma once

#include <snnlib/protos/time_series.pb.h>
#include <google/protobuf/io/zero_copy_stream_impl.h>
#include <google/protobuf/io/coded_stream.h>

#include <fcntl.h>

using namespace google::protobuf::io;



//template <typename T>
//void writeMessages(vector<T> &messages, ostream &out) {
//    //write varint delimiter to buffer
//    google::protobuf::io::OstreamOutputStream zeroOut(&out);
//    google::protobuf::io::CodedOutputStream codedOut(&zeroOut);
//    for(auto it=messages.begin(); it != messages.end(); ++it) {
//        writeMessage(*it, codedOut);
//    }
//}
//
//
//
//
//template <typename T>
//vector<T> readMessages(fstream &in) {
//    IstreamInputStream zeroIn(&in);
//    CodedInputStream codedIn(&zeroIn);
//    vector<T> messages;
//
//    while(!codedIn.ExpectAtEnd()) {
//        T message;
//        readMessage(message, codedIn);
//        messages.push_back(message);
//    }
//    return messages;
//}


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


