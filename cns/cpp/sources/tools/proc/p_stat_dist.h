#pragma once


#include "p_stat_calc.h"

enum  pStatDistOptionIndex { STAT_DIST_PROG_ARG_UNKNOWN, STAT_DIST_PROG_ARG_HELP, STAT_DIST_PROG_ARG_SPIKES, STAT_DIST_PROG_ARG_P_STAT, STAT_DIST_PROG_ARG_JOBS, STAT_DIST_PROG_ARG_OUT_JSON };
const option::Descriptor pStatDistUsage[] =
{
 {STAT_DIST_PROG_ARG_UNKNOWN, 0, "", "",Arg::None, "USAGE: example [options]\n\n"
                                        "Options:" },
 {STAT_DIST_PROG_ARG_SPIKES, 0,"s","spikes",Arg::NonEmpty, "  --spikes, -s  \tLabeled Spikes List to process" },
 {STAT_DIST_PROG_ARG_P_STAT, 0,"p","p-stat",Arg::NonEmpty, "  --p-stat, -p  \tProbability statistics of model to proccess" },
 {STAT_DIST_PROG_ARG_OUT_JSON, 0,"o","output",Arg::NonEmpty, "  --output, -o  \tOutput file in json format" },
 {STAT_DIST_PROG_ARG_JOBS, 0,"j","jobs",Arg::NonEmpty, "  --jobs  -j \tParallel jobs to run (default 1)" },
 {STAT_DIST_PROG_ARG_HELP, 0,"h", "help",Arg::None, "  --help  \tPrint usage and exit." },

 {STAT_DIST_PROG_ARG_UNKNOWN, 0, "", "",Arg::None, "\nExamples:\n"
                               "% Need to fill %" },
 {0,0,0,0,0,0}
};


struct PStatDistanceOpts {
    PStatDistanceOpts() : jobs(1) {}
    string spikes;
    string p_stat;
    string output;
    int jobs;
};

PStatDistanceOpts parsePStatDistOpts(int argc, char **argv) {
    argc-=(argc>0); argv+=(argc>0); // skip program name argv[0] if present
    option::Stats  stats(pStatDistUsage, argc, argv);
    option::Option options[stats.options_max];
    option::Option buffer[stats.buffer_max];

    option::Parser parse(pStatDistUsage, argc, argv, options, buffer);
    if (parse.error())
        exit(1);

    if ((options[STAT_DIST_PROG_ARG_HELP]) || (argc == 0)) {
        option::printUsage(cout, pStatDistUsage);
        exit(0);
    }

    if(options[STAT_DIST_PROG_ARG_SPIKES].count() != 1) {
        cerr << "Inappropriate spikes argument\n";
        exit(1);
    }
    if(options[STAT_DIST_PROG_ARG_P_STAT].count() != 1) {
        cerr << "Inappropriate --p-stat argument\n";
        exit(1);
    }
    for (option::Option* opt = options[STAT_DIST_PROG_ARG_UNKNOWN]; opt; opt = opt->next()) {
      cerr << "Unknown option: " << string(opt->name,opt->namelen) << "\n";
    }
    if(options[STAT_DIST_PROG_ARG_UNKNOWN].count() > 0) exit(1);
    for (int i = 0; i < parse.nonOptionsCount(); ++i) {
      cerr << "Non-options arguments are not supported (" << i << ": " << parse.nonOption(i) << ")\n";
    }
    if(parse.nonOptionsCount()>0) exit(1);

    PStatDistanceOpts opts;
    if(options[STAT_DIST_PROG_ARG_JOBS].count()>0) {
        opts.jobs = atoi(options[STAT_DIST_PROG_ARG_JOBS].arg);
    }
    if(options[STAT_DIST_PROG_ARG_SPIKES].count()>0) {
        opts.spikes = options[STAT_DIST_PROG_ARG_SPIKES].arg;
    }
    if(options[STAT_DIST_PROG_ARG_P_STAT].count()>0) {
        opts.p_stat = options[STAT_DIST_PROG_ARG_P_STAT].arg;
    }
    if(options[STAT_DIST_PROG_ARG_OUT_JSON].count()>0) {
        opts.output = options[STAT_DIST_PROG_ARG_OUT_JSON].arg;
    }
    return opts;
}



void p_stat_dist(int argc, char **argv) {
    PStatDistanceOpts opts = parsePStatDistOpts(argc, argv);
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

    const SpikesList &sl = sp_list->sl;
    double acc_rate = 0;
    for(size_t i = sl.N-1; i>=(sl.N-st.size()) ; i--) {
        acc_rate += sl[i].size()/sp_list->ptl.Tmax;
    }
    double rate = 1000*acc_rate/st.size();


    JsonBox::Value out;
    out["distance_matrix"] = dist.serializeToJson();
    out["mean_rate"] = rate;

    vector<string> labs = sp_list->ptl.getLabelsTimeline();
    JsonBox::Array labs_json;
    for(auto it=labs.begin(); it != labs.end(); ++it) {
        labs_json.push_back(*it);
    }
    out["labels"] = labs_json;

    if(!opts.output.empty()) {
        out.writeToFile(opts.output);
    } else {
        cout << out;
    }

}
