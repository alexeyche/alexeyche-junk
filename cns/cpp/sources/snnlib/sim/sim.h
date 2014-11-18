#pragma once


#include <snnlib/layers/layer.h>
#include <snnlib/util/time_series.h>

#include "network.h"

class Sim: public Printable {
public:
    Sim(const Constants &c);

    ~Sim() {
        if(!statistics_file.empty()) {
            ProtoRw prw(statistics_file, ProtoRw::Write);
            for(auto it=sc.neurons_to_listen.begin(); it != sc.neurons_to_listen.end(); ++it) {
                Neuron *n = accessByGlobalId(*it);
                if(n->stat) {
                    prw.write(n->stat);
                }
            }
        }
        if(!output_spikes_file.empty()){
            ProtoRw prw(output_spikes_file, ProtoRw::Write);
            prw.write(&net.spikes_list);
        }

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
    void setOutputSpikesFile(const string &filename) {
        output_spikes_file = filename;
    }

    Neuron* accessByGlobalId(size_t id) {
        size_t acc = 0;
        for(size_t li=0; li<input_layers.size(); li++) {
            Layer *l = input_layers[li];
            if(id<(acc+l->N)) {
                return input_layers[li]->neurons[id-acc];
            } else {
                acc += l->N;
            }
        }
        for(size_t li=0; li<layers.size(); li++) {
            Layer *l = layers[li];
            if(id<(acc+l->N)) {
                return layers[li]->neurons[id-acc];
            } else {
                acc += l->N;
            }
        }
        cerr << "Can't find neuron with global id " << id << "\n";
        terminate();
    }

    void monitorStat(const string &filename) {
        statistics_file = filename;
        for(auto it=sc.neurons_to_listen.begin(); it != sc.neurons_to_listen.end(); ++it) {
            accessByGlobalId(*it)->enableCollectStatistics();
        }
    }

    void precalculateInputLayerSpikes() {
        if(input_ts.size() == 0) {
            cerr << "Need set input time series to precalculate spikes\n";
            terminate();
        }
        for(double t=0; t<=input_ts.Tmax; t += sc.ts_map_conf.dt) {
            const double &x = input_ts.pop_value();
            //cout << t << ":" << x << "\n";
            for(size_t li=0; li<input_layers.size(); li++) {
                Layer *l = input_layers[li];
                for(size_t ni=0; ni<l->N; ni++) {
                    l->neurons[ni]->attachCurrent(x);
                    l->neurons[ni]->calculateProbability();
                    l->neurons[ni]->calculateDynamics();
                    if(l->neurons[ni]->fired) {
                        net.propagateSpike(l->neurons[ni]->id, t);
                        l->neurons[ni]->fired = 0;
                    }
                }
            }
        }
    }


    void run() {
        precalculateInputLayerSpikes();

    }

    Network net;

    ContLabeledTimeSeries input_ts;

    vector< Layer *> input_layers;
    vector< Layer *> layers;

    string statistics_file;
    string output_spikes_file;

    const SimConfiguration &sc;
};

