
using namespace std;

#include <iostream>
#include <stdlib.h>
#include <fstream>

#include <snnlib/util/optionparser/opt.h>

#include <snnlib/serialize/serialize.h>
#include <snnlib/serialize/proto_rw.h>
#include <snnlib/util/util.h>

enum  optionIndex { ARG_UNKNOWN, ARG_HELP, ARG_INPUT_TS, ARG_OUTPUT_TS };
const option::Descriptor usage[] =
{
 {ARG_UNKNOWN, 0, "", "",Arg::None, "USAGE: example [options]\n\n"
                                        "Options:" },
 {ARG_HELP, 0,"h", "help",Arg::None, "  --help  \tPrint usage and exit." },
 {ARG_INPUT_TS, 0,"i","input-ts",Arg::NonEmpty, "  --input-ts, -i  \tInput time series file." },
 {ARG_OUTPUT_TS, 0,"o","output-ts",Arg::NonEmpty, "  --output-ts, -i  \toutput time series file." },
 {ARG_UNKNOWN, 0, "", "",Arg::None, "\nExamples:\n"
                               "% Need to fill %" },
 {0,0,0,0,0,0}
};



void parseOptions(option::Option* options, option::Stats &stats,  int argc, char **argv) {
    option::Option* buffer  = new option::Option[stats.buffer_max];
    option::Parser parse(usage, argc, argv, options, buffer);
    if (parse.error())
        exit(1);
  
    if (options[ARG_HELP]) { // || argc == 0) {
        option::printUsage(cout, usage);
        exit(0);
    }
    if(options[ARG_INPUT_TS].count() != 1) {
        cerr << "Inappropriate input time series argument\n";
        exit(1);
    }
    if(options[ARG_OUTPUT_TS].count() != 1) {
        cerr << "Inappropriate output time series argument\n";
        exit(1);
    }

    for (option::Option* opt = options[ARG_UNKNOWN]; opt; opt = opt->next()) {
      cerr << "Unknown option: " << string(opt->name,opt->namelen) << "\n";
    }
    if(options[ARG_UNKNOWN].count() > 0) exit(1);

    for (int i = 0; i < parse.nonOptionsCount(); ++i) {
      cerr << "Non-options arguments are not supported (" << i << ": " << parse.nonOption(i) << ")\n";
    }
    if(parse.nonOptionsCount()>0) exit(1);

    delete[] buffer;
}



Protos::LabeledTimeSeries convertUcrTimeSeriesLine(const string &line) {
    vector<string> els = split(line, ' ');
    assert(els.size() > 0);
    
    string lab;
    vector<double> ts_data;
    for(size_t i=0; i<els.size(); i++) {
        trim(els[i]);
        if(!els[i].empty()) {
            if(lab.empty()) {
                std::ostringstream lab_format;
                lab_format << stoi(els[i]);
                lab = lab_format.str();
                continue;
            }
            ts_data.push_back(stof(els[i]));
        }
    }
    Protos::LabeledTimeSeries ts = doubleVectorToLabeledTimeSeries(lab, ts_data);
    return ts;
}

int main(int argc, char **argv) {
    argc-=(argc>0); argv+=(argc>0); // skip program name argv[0] if present
    option::Stats  stats(usage, argc, argv);
    option::Option* options = new option::Option[stats.options_max];
    parseOptions(options, stats, argc, argv); 
    
    ifstream ucr_ts_file(options[ARG_INPUT_TS].arg);
    if (!ucr_ts_file.is_open()) {
        cerr << "Errors while opening " << options[ARG_INPUT_TS].arg << "\n";
        terminate();
    }

    ProtoRw prw(options[ARG_OUTPUT_TS].arg, ProtoRw::Write); 
    string line;
    while ( getline (ucr_ts_file,line) ) {
        Protos::LabeledTimeSeries ts = convertUcrTimeSeriesLine(line);
        prw.write<Protos::LabeledTimeSeries>(ts);
    }
    delete[] options;
}
