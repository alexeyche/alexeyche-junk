
#include <iostream>
#include <stdlib.h>
#include <fstream>

#include <dnn/core.h>
#include <dnn/mpl/mpl.h>
#include <dnn/util/option_parser.h>
#include <dnn/io/stream.h>
#include <dnn/util/time_series.h>

using namespace dnn;
using namespace rapidjson;

const char * usage = R"USAGE(
Matching Pursuit Learning tool

    --input, -i   input time series
    --filter, -f   filter protobin file
    --spikes, -s spikes
    --help,  -h   for this menu
    --config, -c json file with structure like this, to override defaults:

%s
)USAGE";

void printHelp() {
    MPLConfig c;
    Document d = c.serializeToJson();
    StringBuffer sb;
    PrettyWriter<StringBuffer> writer(sb);
    d.Accept(writer);
    string json_struct_str = sb.GetString();
    printf(usage, json_struct_str.c_str());
}

int main(int argc, char **argv) {
    
    if(argc == 1) {
        printHelp();
        return 0; 
    }
    OptionParser optp(argc, argv);
    
    string input_file;
    string spikes_file;
    string config_file;
    string filter_file;
    bool need_help;
    optp.option("--input", "-i", input_file, true);
    optp.option("--help", "-h", input_file, false, true);
    optp.option("--filter", "-f", filter_file);
    optp.option("--spikes", "-s", spikes_file);
    optp.option("--config", "-c", config_file, false);

    if(need_help) {
        printHelp();
        return 0;
    }
    MPLConfig c;
    
    if(!config_file.empty()) {
        std::ifstream ifs(config_file);
        string config_json_str((std::istreambuf_iterator<char>(ifs)),
                 std::istreambuf_iterator<char>());
        Document d = Json::parseString(config_json_str);      
        c.deserializeFromJson(d);
    }
    DoubleMatrix filter;
    if( (!c.learn) || c.continue_learning ) {
        std::ifstream ifs(filter_file);
        Factory::inst().registrationOff();
        DoubleMatrix *f = Stream(ifs, Stream::Binary).readObject<DoubleMatrix>();
        filter = *f;
        delete f;
        Factory::inst().registrationOn();
    }
    std::ifstream ifs(input_file);
    TimeSeries *ts = Stream(ifs, Stream::Binary).readObject<TimeSeries>();
    vector<MPL::FilterMatch> matches = MPL::run(*ts, c, filter);
    return 0;
}