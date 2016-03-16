#pragma once

#include <dnn/protos/config.pb.h>
#include <dnn/util/optional.h>
#include <dnn/sim/sim.h>
#include <dnn/util/protobuf.h>
#include <dnn/util/serial/bin_serial.h>

namespace NDnn {

	struct TModelOptions {
		ui32 Port;
		TOptional<TString> ConfigFile;
		TOptional<TString> InputSpikesFile;
		TOptional<TString> OutputSpikesFile;
		TOptional<TString> StatFile;
		TString Name;
	};

	TModelOptions InitOptions(const int argc, const char** argv, TString name);

	template <typename ... T>
	auto BuildModel(TModelOptions options) {
		auto sim = BuildSim<T...>(options.Port);

		if (options.ConfigFile) {
	    	NDnnProto::TConfig config;
	    	ReadProtoTextFromFile(*options.ConfigFile, config);
	    	sim.Deserialize(config);
	    }

	    if (options.InputSpikesFile) {
    		std::ifstream input(*options.InputSpikesFile, std::ios::binary);
		    TBinSerial serial(input);
	    	sim.SetInputSpikes(serial.ReadObject<TSpikesList>());
	    }

	    return sim;
	}

} // namespace NDnn