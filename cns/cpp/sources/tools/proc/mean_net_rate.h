#pragma once


enum  pStatMeanNetRateOptionIndex { MEAN_RATE_PROG_ARG_UNKNOWN, MEAN_RATE_PROG_ARG_HELP, MEAN_RATE_PROG_ARG_SPIKES, MEAN_RATE_PROG_ARG_NET_NEURONS, MEAN_RATE_PROG_ARG_OUT_JSON };
const option::Descriptor pStatMeanNetRateUsage[] =
{
 {MEAN_RATE_PROG_ARG_UNKNOWN, 0, "", "",Arg::None, "USAGE: example [options]\n\n"
                                        "Options:" },
 {MEAN_RATE_PROG_ARG_SPIKES, 0,"s","spikes",Arg::NonEmpty, "  --spikes, -s  \tLabeled Spikes List to process" },
 {MEAN_RATE_PROG_ARG_NET_NEURONS, 0,"n","net-neurons",Arg::NonEmpty, "  --net-neurons, -n  \tNumber of net neurons" },
 {MEAN_RATE_PROG_ARG_OUT_JSON, 0,"o","output",Arg::NonEmpty, "  --output, -o  \tOutput file in json format" },
 {MEAN_RATE_PROG_ARG_HELP, 0,"h", "help",Arg::None, "  --help  \tPrint usage and exit." },

 {MEAN_RATE_PROG_ARG_UNKNOWN, 0, "", "",Arg::None, "\nExamples:\n"
                               "% Need to fill %" },
 {0,0,0,0,0,0}
};


struct PStatMeanNetOpts {
    PStatMeanNetOpts() {}
    string spikes;
    int net_neurons;
    string output;
};


PStatMeanNetOpts parsePStatMeanNetRateOpts(int argc, char **argv) {
    argc-=(argc>0); argv+=(argc>0); // skip program name argv[0] if present
    option::Stats  stats(pStatMeanNetRateUsage, argc, argv);
    option::Option options[stats.options_max];
    option::Option buffer[stats.buffer_max];

    option::Parser parse(pStatMeanNetRateUsage, argc, argv, options, buffer);
    if (parse.error())
        exit(1);

    if ((options[MEAN_RATE_PROG_ARG_HELP]) || (argc == 0)) {
        option::printUsage(cout, pStatMeanNetRateUsage);
        exit(0);
    }

    if(options[MEAN_RATE_PROG_ARG_SPIKES].count() == 0) {
        cerr << "Need spikes argument\n";
        exit(1);
    }
    if(options[MEAN_RATE_PROG_ARG_NET_NEURONS].count() == 0) {
        cerr << "Need --net-neurons argument\n";
        exit(1);
    }
    for (option::Option* opt = options[MEAN_RATE_PROG_ARG_UNKNOWN]; opt; opt = opt->next()) {
      cerr << "Unknown option: " << string(opt->name,opt->namelen) << "\n";
    }
    if(options[MEAN_RATE_PROG_ARG_UNKNOWN].count() > 0) exit(1);
    for (int i = 0; i < parse.nonOptionsCount(); ++i) {
      cerr << "Non-options arguments are not supported (" << i << ": " << parse.nonOption(i) << ")\n";
    }
    if(parse.nonOptionsCount()>0) exit(1);

    PStatMeanNetOpts opts;
    if(options[MEAN_RATE_PROG_ARG_SPIKES].count()>0) {
        opts.spikes = options[MEAN_RATE_PROG_ARG_SPIKES].last()->arg;
    }
    if(options[MEAN_RATE_PROG_ARG_NET_NEURONS].count()>0) {
        opts.net_neurons = atoi(options[MEAN_RATE_PROG_ARG_NET_NEURONS].last()->arg);
    }
    if(options[MEAN_RATE_PROG_ARG_OUT_JSON].count()>0) {
        opts.output = options[MEAN_RATE_PROG_ARG_OUT_JSON].last()->arg;
    }
    return opts;
}



void mean_net_rate(int argc, char **argv) {
    PStatMeanNetOpts opts = parsePStatMeanNetRateOpts(argc, argv);

    LabeledSpikesList* sp_list;
    {
        ProtoRw prw(opts.spikes, ProtoRw::Read);
        sp_list = prw.read<LabeledSpikesList>();
    }


    const SpikesList &sl = sp_list->sl;
    double acc_rate = 0;
    for(size_t i = sl.N-1; i>=(sl.N-opts.net_neurons); i--) {
        acc_rate += sl[i].size()/sp_list->ptl.Tmax;
    }
    double rate = 1000*acc_rate/opts.net_neurons;

    JsonBox::Value out;
    out["mean_rate"] = rate;

    if(!opts.output.empty()) {
        out.writeToFile(opts.output);
    } else {
        cout << out << "\n";
    }

}
