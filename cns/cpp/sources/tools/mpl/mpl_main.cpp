
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
    MatchingPursuitConfig c;
    ostringstream s;
    Stream(s, Stream::Text).writeObject(&c);
    string json_str = s.str();
    printf(usage, json_str.c_str());
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
    bool need_help = false;
    optp.option("--help", "-h", need_help, false, true);
    if(need_help) {
        printHelp();
        return 0;
    }
    optp.option("--input", "-i", input_file, true);
    optp.option("--filter", "-f", filter_file);
    optp.option("--spikes", "-s", spikes_file);
    optp.option("--config", "-c", config_file, false);

    
    MatchingPursuitConfig c;
    
    if(!config_file.empty()) {
        std::ifstream ifs(config_file);
        Stream(ifs, Stream::Text).readObject<MatchingPursuitConfig>(&c);        
    }
    MatchingPursuit mpl(c);

    if( (fileExists(filter_file)) && ( (!c.learn) || c.continue_learning )) {
        std::ifstream ifs(filter_file);
        Factory::inst().registrationOff();
        DoubleMatrix *f = Stream(ifs, Stream::Binary).readObject<DoubleMatrix>();
        
        mpl.setFilter(*f);
        
        delete f;
        Factory::inst().registrationOn();
    }

    std::ifstream ifs(input_file);
    TimeSeries *ts = Stream(ifs, Stream::Binary).readObject<TimeSeries>();
    
    mpl.run(*ts, 0);

    std::ofstream ofs(filter_file);
    DoubleMatrix f = mpl.getFilter();
    Stream(ofs, Stream::Binary).writeObject(&f);
    return 0;
}