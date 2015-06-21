
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
    --dim,   -d   index of dimension of input time series to use
    --restored,   -r   calculate restored time series and print error (not required)
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
    int dimension = 0;
    string restored_ts;
    optp.option("--help", "-h", need_help, false, true);
    if(need_help) {
        printHelp();
        return 0;
    }
    optp.option("--input", "-i", input_file, true);
    optp.option("--filter", "-f", filter_file);
    optp.option("--spikes", "-s", spikes_file, false);
    optp.option("--config", "-c", config_file, false);
    optp.option("--dim", "-d", dimension, false);
    optp.option("--restored", "-r", restored_ts, false);

    
    MatchingPursuitConfig c;
    
    if(!config_file.empty()) {
        std::ifstream ifs(config_file);
        Stream(ifs, Stream::Text).readObject<MatchingPursuitConfig>(&c);        
    }
    MatchingPursuit mpl(c);

    if( (fileExists(filter_file)) && ( (!c.learn) || c.continue_learning )) {
        cout << "Reading filter from " << filter_file << "\n";
        std::ifstream ifs(filter_file);
        Factory::inst().registrationOff();
        DoubleMatrix *f = Stream(ifs, Stream::Binary).readObject<DoubleMatrix>();
        
        mpl.setFilter(*f);
        
        delete f;
        Factory::inst().registrationOn();
    }

    std::ifstream ifs(input_file);
    TimeSeries *ts = Stream(ifs, Stream::Binary).readObject<TimeSeries>();
    if(dimension>=ts->dim()) {
        throw dnnException() << "Can't find dimension with index " << dimension << " in input time series\n";
    }
    MatchingPursuit::MPLReturn r = mpl.run(*ts, dimension);
    if(c.learn) {
        std::ofstream ofs(filter_file);
        DoubleMatrix f = mpl.getFilter();
        Stream(ofs, Stream::Binary).writeObject(&f);
    }
    if(!spikes_file.empty()) {
        std::ofstream ofs(spikes_file);
        Stream s(ofs, Stream::Binary);
        for(auto &m: r.matches) {
            s.writeObject(&m);    
        }
    }
    
    

    if(!restored_ts.empty()) {
        vector<double> v = mpl.restore(r.matches);
        TimeSeries ts_rest(v);
        std::ofstream s(restored_ts);
        Stream(s, Stream::Binary).writeObject(&ts_rest);
        double acc_error = 0;
        for(size_t vi=0; vi<ts->length(); ++vi) {
            const double& orig_val = ts->getValueAtDim(vi, dimension);
            double rest_val;
            if(vi < v.size()) {
                rest_val = v[vi];
            } else {
                rest_val = 0.0;
            }
            acc_error += (orig_val - rest_val)*(orig_val - rest_val);
        }
        cout << "\nAccumulated error: " << 1000*acc_error/ts->length() << "\n";
        // Document d;
        // d.SetObject();
        // d.AddMember("accum_error", acc_error, d.GetAllocator());
        // cout << Json::stringify(d) << "\n";
    }
    return 0;
}