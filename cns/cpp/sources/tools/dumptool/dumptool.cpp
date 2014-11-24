using namespace std;

#include <iostream>
#include <stdlib.h>
#include <fstream>

#include <snnlib/util/optionparser/opt.h>

#include <snnlib/serialize/serialize.h>
#include <snnlib/serialize/proto_rw.h>
#include <snnlib/util/util.h>

#include "ucr_ts.h"


enum  optionIndex { ARG_UNKNOWN, ARG_HELP, ARG_UCR_TS, ARG_LAB_TS_PB, ARG_PB };
const option::Descriptor usage[] =
{
 {ARG_UNKNOWN, 0, "", "",Arg::None, "USAGE: example [options]\n\n"
                                        "Options:" },
 {ARG_HELP, 0,"h", "help",Arg::None, "  --help  \tPrint usage and exit." },
 {ARG_UCR_TS, 0,"","ucr-ts",Arg::NonEmpty, "  --ucr-ts  \tInput time series file from UCR dataset." },
 {ARG_LAB_TS_PB, 0,"o","lab-ts-pb",Arg::NonEmpty, "  --lab-ts-pb  \tLabeledTimeSeries protobuf file (for dump or for write)." },
 {ARG_PB, 0,"i","pb",Arg::NonEmpty, "  --pb  \tAny protobuf file to dump" },
 {ARG_UNKNOWN, 0, "", "",Arg::None, "\nExamples:\n"
                               "dumptool -i file.pb     To dump any protobuyf from this library\n"
                               "dumptool --lab-ts-pb timeseries.pb     To dump protobuf binary file to stdout\n"
                               "dumptool --ucr-ts synthetic_control_TRAIN --lab-ts-pb timeseries.pb\n"
},
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

int main(int argc, char **argv) {
    argc-=(argc>0); argv+=(argc>0); // skip program name argv[0] if present
    option::Stats  stats(usage, argc, argv);
    option::Option* options = new option::Option[stats.options_max];
    parseOptions(options, stats, argc, argv);


    if(options[ARG_UCR_TS].count()>0) {
        ifstream ucr_ts_file(options[ARG_UCR_TS].arg);
        if (!ucr_ts_file.is_open()) {
            cerr << "Errors while opening " << options[ARG_UCR_TS].arg << "\n";
            terminate();
        }
        if(options[ARG_LAB_TS_PB].count() != 1) {
            cerr << "Need one --lab-ts-pb option for output protobuf file\n";
            terminate();
        }
        LabeledTimeSeriesList list_of_lts;
        string line;
        int i =0;
        while ( getline (ucr_ts_file,line) ) {
            LabeledTimeSeries ts = convertUcrTimeSeriesLine(line);
            list_of_lts.push_back(ts);
        }
        ProtoRw prw(options[ARG_LAB_TS_PB].arg, ProtoRw::Write);
        prw.write(&list_of_lts);
    } else
    if(options[ARG_PB].count() > 0) {
        ProtoRw prw(options[ARG_PB].arg, ProtoRw::Read);
        while(true) {
            vector<Serializable*> v = prw.readAny(true);
            if(v.size() == 0) break;
        }
    } else {
        option::printUsage(cout, usage);
    }
    delete[] options;
}
