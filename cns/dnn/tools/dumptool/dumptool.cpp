
#include <iostream>
#include <fstream>

#include <dnn/protos/options.pb.h>
#include <dnn/util/proto_options.h>
#include <dnn/util/ts/time_series.h>
#include <dnn/util/ts/spikes_list.h>

using namespace NDnn;

#include "dumptool.h"


int main(int argc, const char** argv) {
	TProtoOptions<NDnnProto::TDumptoolOptions> clOptions(argc, argv, "Dynamic neural network dumptool");

    NDnnProto::TDumptoolOptions options;
    if (!clOptions.Parse(options)) {
        return 0;
    }

    std::fstream input(options.input(), std::ios::in | std::ios::binary);
    if (DumpEntity<TTimeSeries>(input)) {
    	return 0;
    }
	if (DumpEntity<TSpikesList>(input)) {
    	return 0;
    }

    return 1;
}
