#pragma once

#include <snnlib/core.h>
#include <snnlib/protos/time_series.pb.h>
#include <snnlib/serialize/serialize.h>

class TimeSeries {
public:    
    vector<double> data;
};


class LabeledTimeSeries: TimeSeries {
public:    
    string lab;
};


class LabeledTimeSeriesList {
public:    
    LabeledTimeSeriesList(const string &fname) {
        deserialize(fname);
    }

    vector<LabeledTimeSeries> ts;

    void deserialize(const string &fname) {
        fstream f(fname, ios::in |  ios::binary);
        auto m = readMessages<Protos::LabeledTimeSeries>(f);

    }
};



