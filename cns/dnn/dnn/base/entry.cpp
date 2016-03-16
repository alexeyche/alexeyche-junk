#include "entry.h"

#include <dnn/util/string.h>
#include <dnn/util/proto_options.h>
#include <dnn/protos/options.pb.h>

#include <cstdlib>

namespace NDnn {

	TModelOptions InitOptions(const int argc, const char** argv, TString name) {
		TProtoOptions<NDnnProto::TDnnOptions> clOptions(argc, argv, NStr::TStringBuilder() << "Dynamic neural network model, " << name);
		NDnnProto::TDnnOptions options;
		if (!clOptions.Parse(options)) {
			exit(1);
		}
	    
	    if (options.verbose()) {
	        TLog::Instance().SetLogLevel(TLog::DEBUG_LEVEL);
	    }

	    TModelOptions opts;
	    opts.Name = name;

	    if (options.has_config()) {
	    	opts.ConfigFile = options.config();
	    	
	    	NDnnProto::TConfig config;
	        ReadProtoTextFromFile(*opts.ConfigFile, config);
	        opts.Port = config.simconfiguration().port();
	    }

	    if (options.has_inputspikes()) {
	    	opts.InputSpikesFile = options.inputspikes();
	    }

	    if (options.has_port()) {
	        opts.Port = options.port();
	    }

	    if (options.has_outputspikes()) {
	    	opts.OutputSpikesFile = options.outputspikes();
	    }
	    return opts;
	}


} // namespace NDnn