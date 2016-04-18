
#include <iostream>

#include <dnn/base/entry.h>

#include <dnn/neuron/integrate_and_fire.h>
#include <dnn/neuron/spike_sequence_neuron.h>
#include <dnn/synapse/synapse.h>
#include <dnn/protos/options.pb.h>

using namespace NDnn;

int main(int argc, const char** argv) {
    auto opts = InitOptions(argc, argv, "TestModel");

    auto sim = BuildModel<
        TLayer<TSpikeSequenceNeuron, 100>,
        TLayer<TIntegrateAndFire, 100, TNeuronConfig<TBasicSynapse, TDeterm>>
    >(opts);

    // sim.ListenBasicStats<1, 55>(0, 1000);

    sim.Run();

    if (opts.OutputSpikesFile) {
        sim.SaveSpikes(*opts.OutputSpikesFile);
    }

    if (opts.StatFile) {
        sim.SaveStat(*opts.StatFile);
    }

    if (opts.ModelSave) {
        sim.SaveModel(*opts.ModelSave);
    }
    return 0;
}
