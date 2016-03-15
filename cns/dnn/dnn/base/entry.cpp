#include "entry.h"

#include <dnn/util/string.h>
#include <dnn/util/protobuf.h>
#include <dnn/util/proto_options.h>
#include <dnn/protos/options.pb.h>


namespace NDnn {

	TModelOptions InitOptions(const int argc, const char** argv, TString name) {
		TProtoOptions<NDnnProto::TDnnOptions> clOptions(argc, argv, NStr::TStringBuilder() << "Dynamic neural network model " << name);
		NDnnProto::TDnnOptions options;
	    ENSURE(clOptions.Parse(options), "Options parsing error");
	    
	    if (options.verbose()) {
	        TLog::Instance().SetLogLevel(TLog::DEBUG_LEVEL);
	    }

	    TModelOptions opts;
	    opts.Name = name;

	    if (options.has_config()) {
	        opts.Config.emplace(NDnnProto::TConfig());
	        ReadProtoTextFromFile(options.config(), *opts.Config);
	        opts.Port = opts.Config->simconfiguration().port();
	    }

	    if (options.has_port()) {
	        opts.Port = options.port();
	    }
	    return opts;
	}


} // namespace NDnn