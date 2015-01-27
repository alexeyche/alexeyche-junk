
#include <iostream>
#include <stdlib.h>
#include <fstream>

#include <snnlib/util/optionparser/opt.h>

#include <snnlib/serialize/serialize.h>
#include <snnlib/serialize/proto_rw.h>
#include <snnlib/util/util.h>
#include <snnlib/util/gammatone_fb.h>
#include <snnlib/util/time_series.h>


enum  optionIndex { ARG_UNKNOWN, ARG_HELP,  ARG_INPUT, ARG_OUTPUT };
const option::Descriptor usage[] =
{
 {ARG_UNKNOWN, 0, "", "",Arg::None, "USAGE: example [options]\n\n"
                                        "Options:" },
 {ARG_HELP, 0,"h", "help",Arg::None, "  --help  \tPrint usage and exit." },

 {ARG_OUTPUT, 0,"o","output",Arg::NonEmpty, "  --output  \tOutput time series" },
 {ARG_INPUT, 0,"i","input",Arg::NonEmpty, "  --input  \tInput time series" },
 {ARG_UNKNOWN, 0, "", "",Arg::None, "\nExamples:\n"},
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

    if((options[ARG_INPUT].count()<=0)||(options[ARG_OUTPUT].count()<=0)) {
        option::printUsage(cout, usage);
        return 1;
    }
    string input_ts = options[ARG_INPUT].last()->arg;
    string output_ts = options[ARG_OUTPUT].last()->arg;

    LabeledTimeSeriesList* ts_list;
    {
        ProtoRw prw(input_ts, ProtoRw::Read);
        ts_list = prw.read<LabeledTimeSeriesList>();
    }

    GammatoneFilter gf(GammatoneFilter::Options::Full);

    gf.calc(ts_list->ts[0].ts.data, 1000, 100, 0);
    //print_vector<double>(gf.membrane, cout, ", ");
    return 0;
}


