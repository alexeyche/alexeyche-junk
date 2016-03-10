
#include <iostream>

#include <dnn/util/log/log.h>
#include <dnn/sim/sim.h>
#include <dnn/neuron/config.h>
#include <dnn/synapse/basic_synapse.h>
#include <dnn/activation/determ.h>
#include <dnn/neuron/integrate_and_fire.h>
#include <dnn/synapse/synapse.h>

using namespace NDnn;

int main(int argc, const char** argv) {
	TLog::Instance().SetLogLevel(TLog::DEBUG_LEVEL);

	auto sim = BuildSim<
		TLayer<TIntegrateAndFire, 100, TNeuronConfig<TBasicSynapse, TDeterm>>,
		TLayer<TIntegrateAndFire, 100>
	>();

	sim.SerializeToTextStream(std::cout);
    return 0;
}
