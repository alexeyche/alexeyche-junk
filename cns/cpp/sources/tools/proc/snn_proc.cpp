

#include <snnlib/util/optionparser/opt.h>
#include <snnlib/serialize/proto_rw.h>
#include <snnlib/neurons/neuron_stat.h>
#include <snnlib/util/spikes_list.h>
#include <snnlib/base.h>
#include <snnlib/util/matrix.h>


#include "p_stat_calc.h"

enum  optionIndex { ARG_UNKNOWN, ARG_HELP, ARG_SPIKES, ARG_P_STAT, ARG_JOBS };
const option::Descriptor usage[] =
{
 {ARG_UNKNOWN, 0, "", "",Arg::None, "USAGE: example [options]\n\n"
                                        "Options:" },
 {ARG_SPIKES, 0,"s","spikes",Arg::NonEmpty, "  --spikes, -s  \tLabeled Spikes List to process" },
 {ARG_P_STAT, 0,"p","p-stat",Arg::NonEmpty, "  --p-stat, -p  \tProbability statistics of model to proccess" },
 {ARG_JOBS, 0,"j","jobs",Arg::NonEmpty, "  --jobs  -j \tParallel jobs to run (default 1)" },
 {ARG_HELP, 0,"h", "help",Arg::None, "  --help  \tPrint usage and exit." },

 {ARG_UNKNOWN, 0, "", "",Arg::None, "\nExamples:\n"
                               "% Need to fill %" },
 {0,0,0,0,0,0}
};

struct SnnProcOpts {
    SnnProcOpts() : jobs(1) {}
    string spikes;
    string p_stat;
    int jobs;
};

SnnProcOpts parseOptions(int argc, char **argv) {
    argc-=(argc>0); argv+=(argc>0); // skip program name argv[0] if present
    option::Stats  stats(usage, argc, argv);
    option::Option options[stats.options_max];
    option::Option buffer[stats.buffer_max];

    option::Parser parse(usage, argc, argv, options, buffer);
    if (parse.error())
        exit(1);

    if ((options[ARG_HELP]) || (argc == 0)) {
        option::printUsage(cout, usage);
        exit(0);
    }

    if(options[ARG_SPIKES].count() != 1) {
        cerr << "Inappropriate spikes argument\n";
        exit(1);
    }
    if(options[ARG_P_STAT].count() != 1) {
        cerr << "Inappropriate --p-stat argument\n";
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

    SnnProcOpts opts;
    if(options[ARG_JOBS].count()>0) {
        opts.jobs = atoi(options[ARG_JOBS].arg);
    }
    if(options[ARG_SPIKES].count()>0) {
        opts.spikes = options[ARG_SPIKES].arg;
    }
    if(options[ARG_P_STAT].count()>0) {
        opts.p_stat = options[ARG_P_STAT].arg;
    }
    return opts;
}





int main(int argc, char **argv) {
    SnnProcOpts opts = parseOptions(argc, argv);
    vector<NeuronStat*> st;
    {
        ProtoRw prw(opts.p_stat, ProtoRw::Read);
        st = prw.readAll<NeuronStat>();
    }

    LabeledSpikesList* sp_list;
    {
        ProtoRw prw(opts.spikes, ProtoRw::Read);
        sp_list = prw.read<LabeledSpikesList>();
    }
    assert(st.size() > 0);

    size_t p_size = st[0]->p.size();

    vector<IndexSlice> patterns;

    const vector<double> &tl = sp_list->ptl.timeline;
    const double &dt = sp_list->ptl.dt;

    double t=0;
    for(auto it=tl.begin(); it != tl.end(); ++it) {
        patterns.push_back( IndexSlice((size_t)(t/dt), min(p_size, (size_t)((*it)/dt) )) );
        //cout << "adding pattern [" << patterns.back().from << ", " << patterns.back().to << ")\n";
        t = *it;
    }

    DoubleMatrix dist = calcPStatDistance(st, patterns, opts.jobs);
    vector<string> labs = sp_list->ptl.getLabelsTimeline();
    dist.printR(labs, labs);

    return 0;
}

