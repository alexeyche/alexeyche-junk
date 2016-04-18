
#include <iostream>
#include <fstream>

#include <dnn/protos/options.pb.h>
#include <dnn/protos/config.pb.h>
#include <dnn/util/proto_options.h>
#include <dnn/util/ts/time_series.h>
#include <dnn/util/ts/spikes_list.h>
#include <dnn/util/serial/bin_serial.h>
#include <dnn/util/log/log.h>
#include <dnn/util/stat_gatherer.h>

using namespace NDnn;

#include "dumptool.h"


int main(int argc, const char** argv) {
	TProtoOptions<NDnnProto::TDumptoolOptions> clOptions(argc, argv, "Dynamic neural network dumptool");

    NDnnProto::TDumptoolOptions options;
    if (!clOptions.Parse(options)) {
        return 0;
    }
    
    TLog::Instance().SetLogLevel(TLog::DEBUG_LEVEL);

    std::ifstream input(options.input(), std::ios::binary);
    TBinSerial serial(input);
    switch (serial.ReadProtobufType()) {
        case EProto::TIME_SERIES:
            DumpEntity<TTimeSeries>(serial);
            break;
        case EProto::SPIKES_LIST:
            DumpEntity<TSpikesList>(serial);
            break;
        case EProto::STATISTICS:
            DumpEntities<TStatistics>(serial);
            break;
        case EProto::CONFIG:
            {
                NDnnProto::TConfig config;
                serial.ReadProtobufMessage(config);
                std::cout << config.DebugString();
            }
            break;
        default:
            throw TDnnException() << "Failed to recognize protobuf type";
    }
    return 0;
}
