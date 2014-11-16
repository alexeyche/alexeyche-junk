#pragma once


#include <snnlib/layers/layer.h>
#include <snnlib/util/time_series.h>

#include "network_stat.h"

class Sim: public Printable {
public: 
    Sim(const Constants &c);
    ~Sim() {
        input_layers.clear();
        layers.clear();
    }
    void print(std::ostream& str) const {
        for(auto it=input_layers.begin(); it!=input_layers.end(); ++it) {
            str << **it;
        }
        for(auto it=layers.begin(); it!=layers.end(); ++it) {
            str << **it;
        }
    }
    void setInputTimeSeries(LabeledTimeSeriesList l) {
        input_ts = l;
        size_t fullLenght = input_ts.getFullSampleLength();
        cout << fullLenght << "\n";
    }   
    void precalculateInputLayerSpikes() {
        if(input_ts.size() == 0) {
            cerr << "Need set input time series to precalculate spikes\n";
            terminate(); 
        }
    } 
    void run() {

    }
    double Tmax;

    NetworkStat net;

    LabeledTimeSeriesList input_ts;

    vector<Layer*> input_layers;
    vector<Layer*> layers;


    const SimConfiguration &sc;
};

