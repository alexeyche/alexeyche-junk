
#include <iostream>

#include <dnn/util/log/log.h>
#include <dnn/sim/sim.h>
#include <dnn/neuron/config.h>
#include <dnn/synapse/basic_synapse.h>
#include <dnn/activation/determ.h>
#include <dnn/neuron/integrate_and_fire.h>
#include <dnn/synapse/synapse.h>

#include <dnn/protos/config.pb.h>
#include <dnn/protos/options.pb.h>
#include <dnn/util/proto_options.h>
#include <dnn/util/protobuf.h>

using namespace NDnn;
using namespace NDnnProto;

int main(int argc, const char** argv) {
	TProtoOptions<TDnnOptions> clOptions(argc, argv, "Dynamic neural network tool");

    TDnnOptions options;
    if (!clOptions.Parse(options)) {
        return 0;
    }
    if (options.verbose()) {
        TLog::Instance().SetLogLevel(TLog::DEBUG_LEVEL);
    }

    auto sim = BuildSim<
		TLayer<TIntegrateAndFire, 5, TNeuronConfig<TBasicSynapse, TDeterm>>,
		TLayer<TIntegrateAndFire, 5>
	>();


    if (options.has_config()) {
    	TConfig config;
    	ReadProtoTextFromFile(options.config(), config);
    	sim.Deserialize(config);
    }
	
	TConfig conf = sim.Serialize();
	std::cout << conf.DebugString();
    return 0;
}