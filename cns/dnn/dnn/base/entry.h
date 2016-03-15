#pragma once

#include <dnn/protos/config.pb.h>
#include <dnn/util/optional.h>
#include <dnn/sim/sim.h>


namespace NDnn {

	struct TModelOptions {
		ui32 Port;
		TOptional<NDnnProto::TConfig> Config;
		TString Name;
	};

	TModelOptions InitOptions(const int argc, const char** argv, TString name);

	template <typename ... T>
	auto BuildModel(TModelOptions options) {
		
		auto sim = BuildSim<T...>(options.Port);

	    if (options.Config) {
	    	sim.Deserialize(*options.Config);
	    }
	    return sim;
	}

} // namespace NDnn