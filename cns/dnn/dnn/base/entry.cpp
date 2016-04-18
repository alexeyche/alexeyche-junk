#include "entry.h"

#include <dnn/util/string.h>
#include <dnn/util/proto_options.h>
#include <dnn/protos/options.pb.h>

#include <cstdlib>

namespace NDnn {

	TModelOptions InitOptions(const int argc, const char** argv, TString name, std::set<int> fields) {
		TProtoOptions<NDnnProto::TDnnOptions> clOptions(argc, argv, NStr::TStringBuilder() << "Dynamic neural network model, " << name, fields);
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

	    if (options.has_output()) {
	    	opts.OutputSpikesFile = options.output();
	    }

	    if (options.has_stat()) {
	    	opts.StatFile = options.stat();
	    }

	    if (options.has_jobs()) {
	    	opts.Jobs = options.jobs();
	    }
	    if (options.has_load()) {
	    	opts.ModelLoad = options.load();
	    }
	    
	    if (options.has_save()) {
	    	opts.ModelSave = options.save();
	    }
	    return opts;
	}


} // namespace NDnn