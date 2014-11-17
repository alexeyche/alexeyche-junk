#pragma once


#include <snnlib/layers/layer.h>
#include <snnlib/util/time_series.h>

#include "network.h"

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
        input_ts = ContLabeledTimeSeries(l, sc.ts_map_conf.dt);
    }
    void precalculateInputLayerSpikes() {
        if(input_ts.size() == 0) {
            cerr << "Need set input time series to precalculate spikes\n";
            terminate();
        }
    }
    void run() {
        double dt = 1.0;
        Tmax=1000;
        for(double t=0; t<Tmax; t += dt) {
            const double &x = input_ts.pop_value();
            for(size_t li=0; li<input_layers.size(); li++) {
                Layer *l = input_layers[li];
                for(size_t ni=0; ni<l->N; ni++) {
                    l->neurons[ni]->attachCurrent(x);
                    l->neurons[ni]->calculateProbability();
                    l->neurons[ni]->calculateDynamics();
                }


            }
        }
    }
    double Tmax;

    Network net;

    ContLabeledTimeSeries input_ts;

    vector< Layer *> input_layers;
    vector< Layer *> layers;


    const SimConfiguration &sc;
};

