#pragma once


#include <snnlib/layers/layer.h>
#include <snnlib/util/time_series.h>
#include <snnlib/util/spinning_barrier.h>

#include "network.h"

#define P( condition ) {if( (condition) != 0 ) { printf( "\n FAILURE in %s, line %d\n", __FILE__, __LINE__ );exit( 1 );}}


#include "sim_neuron.h"

class Sim: public Printable {
public:
    Sim(Constants &c, size_t _jobs=1);
    Sim(size_t _jobs=1);
    void construct(Constants &c);

    #define CHECK_CONSTRUCT() \
        if(!constructed) {  \
            cerr << "Need construct sim with constants\n"; \
            terminate(); \
        }\

    struct SimWorker {
        Sim *s;
        size_t thread_id;
        int first;
        int last;
    };

    ~Sim() {
        if(!statistics_file.empty()) {
            ProtoRw prw(statistics_file, ProtoRw::Write);
            SerialPack st;
            for(auto it=sc.neurons_to_listen.begin(); it != sc.neurons_to_listen.end(); ++it) {
                Neuron *n = accessByGlobalId(*it);
                n->saveStat(st);
            }
            prw.write(st);

        }
        if(!output_spikes_file.empty()){
            ProtoRw prw(output_spikes_file, ProtoRw::Write);
            prw.write(&net.spikes_list);
        }

        layers.clear();
    }

    void loadModel(string f) {
        ProtoRw rw(f,ProtoRw::Read);
        if(Constants::IsGlobalInstanceCreated()) {
            Constants* c_serial = rw.read()->castSerializable<Constants>();
            if(*c_serial != Constants::globalInstance()) {
                cerr << "Constants in model doesn't equals to constants in -c option\n";
                terminate();
            }
        } else {
            Constants* c_serial = rw.read()->castSerializable<Constants>();
            construct(*c_serial);
        }
        for(auto it = layers.begin(); it != layers.end(); ++it) {
            (*it)->loadModel(rw);
        }
    }
    void saveModel(string f) {
        ProtoRw rw(f,ProtoRw::Write);
        rw.write(&rg.mut_C());
        for(auto it = layers.begin(); it != layers.end(); ++it) {
            (*it)->saveModel(rw);
        }
    }
    void print(std::ostream& str) const {
        for(auto it=layers.begin(); it!=layers.end(); ++it) {
            str << **it;
        }
        str << net;
    }
    void setInputTimeSeries(LabeledTimeSeriesList l) {
        CHECK_CONSTRUCT()
        input_ts_list = l;
        input_ts = ContLabeledTimeSeries(l, sc.ts_map_conf.dt);
    }

    void setInputSpikesList(SpikesList l) {
        CHECK_CONSTRUCT()
        if(l.N < input_neurons_count) {
            cerr << "Input spikes list is inconsistent with const.json\n";
            terminate();
        }
        for(size_t ni=0; ni<input_neurons_count; ni++) {
            net.spikes_list[ni] = l[ni];
        }
    }

    void setOutputSpikesFile(const string &filename) {
        CHECK_CONSTRUCT()
        output_spikes_file = filename;
    }

    Neuron* accessByGlobalId(size_t id) {
        CHECK_CONSTRUCT()
        size_t acc = 0;
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

    void resetSim(bool reset_input_stat=false) {
        CHECK_CONSTRUCT()
        net.reset(reset_input_stat);
        if(input_ts_list.size()>0){
            input_ts = ContLabeledTimeSeries(input_ts_list, sc.ts_map_conf.dt);
        }
    }

    void monitorStat(const string &filename) {
        CHECK_CONSTRUCT()
        statistics_file = filename;
        for(auto it=sc.neurons_to_listen.begin(); it != sc.neurons_to_listen.end(); ++it) {
            accessByGlobalId(*it)->enableCollectStatistics();
        }
    }
    void setTlimit(double _T_limit) {
        T_limit = _T_limit;
    }

    static void* runWorker(void *content);
    static inline void simStep(SimWorker *sw, const double &t);
    void runSimOnSubset(size_t left_neuron_id, size_t right_neuron_id, void* (*sim_func)(void* content));
    void precalculateInputSpikes();
    void run();

    double Tmax;
    double T_limit;

    size_t jobs;
    Network net;

    ContLabeledTimeSeries input_ts;

    size_t input_layers_count;

    size_t input_neurons_count;
    size_t net_neurons_count;

    vector< Layer *> layers;
    vector<SimNeuron> sim_neurons;

    string statistics_file;
    string output_spikes_file;

    SimConfiguration sc;
    RuntimeGlobals rg;
    bool constructed;

    LabeledTimeSeriesList input_ts_list;
};

