
#include <iostream>

#include <dnn/util/log/log.h>
#include <dnn/sim/sim.h>
#include <dnn/neuron/config.h>
#include <dnn/synapse/basic_synapse.h>
#include <dnn/activation/determ.h>
#include <dnn/neuron/integrate_and_fire.h>
#include <dnn/neuron/spike_sequence_neuron.h>
#include <dnn/synapse/synapse.h>

#include <dnn/base/entry.h>

using namespace NDnn;

int main(int argc, const char** argv) {
    auto opts = InitOptions(argc, argv, "TestModel");
    
    auto sim = BuildModel<
        TLayer<TSpikeSequenceNeuron, 100>, 
        TLayer<TIntegrateAndFire, 100, TNeuronConfig<TBasicSynapse, TDeterm>>
    >(opts);

    sim.Run();

    if (opts.OutputSpikesFile) {
		std::ofstream output(*opts.OutputSpikesFile, std::ios::binary);
	    TBinSerial serial(output);
		serial.WriteObject<TSpikesList>(sim.GetSpikes());
    }

    return 0;
}
